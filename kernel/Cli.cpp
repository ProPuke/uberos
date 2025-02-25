#include "Cli.hpp"

#include <drivers/Graphics.hpp>
#include <drivers/Interrupt.hpp>
#include <drivers/Processor.hpp>
#include <drivers/Serial.hpp>

#include <kernel/console.hpp>
#include <kernel/Driver.hpp>
#include <kernel/drivers.hpp>
#include <kernel/Log.hpp>
#include <kernel/memory.hpp>
#include <kernel/mmio.hpp>
#include <kernel/Process.hpp>

#include <common/Box.hpp>

static Log log("");

namespace cli {
	namespace {
		const char *format_object = "\x1b[33m";
		const char *format_verb   = "\x1b[1;36m";
		const char *format_param  = "\x1b[1;35m";;
		const char *format_cwd    = "\x1b[32m";
		const char *format_input  = "\x1b[1;37m";
		const char *format_none   = "\x1b[0m";

		bool is_whitespace(C8 c){
			switch(c){
				case ' ':
				case '\t':
				case '\r':
				case '\n':
					return true;
				break;
				default:
					return false;
			}
		}

		Box<char> resolve_path(const char *pathA, const char *pathB){
			if(!*pathB) return strcpy(new C8[strlen(pathA)+1], pathA);

			if(!*pathA) pathA = "";

			if(pathB[0]=='/'){
				return resolve_path("", pathB+1);

			}else if(pathB[0]=='.'&&pathB[1]=='.'&&(pathB[2]=='/'||pathB[2]=='\0')){
				U32 length = strlen(pathA);
				while(length>0&&pathA[length-1]!='/') length--;
				if(length>0) length--;

				Box temp = (char*)memcpy(new C8[length+1], pathA, length);
				temp.get()[length] = '\0';
				if(!pathB[2]) return temp.release();

				return resolve_path(temp.get(), pathB+3);

			}else if(pathB[0]=='.'&&(pathB[1]=='/'||pathB[1]=='\0')){
				return resolve_path(pathA, pathB+(pathB[1]=='/'?2:1));

			}else{
				U32 length = 0;
				while(pathB[length]&&pathB[length]!='/') length++;

				U32 aLength = strlen(pathA);

				auto temp = (char*)memcpy(new C8[aLength+(aLength?1:0)+length+1], pathA, aLength);
				if(aLength) temp[aLength] = '/';

				memcpy(temp+aLength+(aLength?1:0), pathB, length);
				temp[aLength+(aLength?1:0)+length] = '\0';

				if(pathB[length]=='/'){
					auto result = resolve_path(temp, pathB+length+1);
					delete temp;
					return result;
				}else{
					return temp;
				}
			}
		}
	}

	struct VerbObject {
		/**/ VerbObject(){}
		virtual /**/~VerbObject(){}
		virtual Box<VerbObject> clone() = 0;

		virtual void get_children(void(*callback)(const char *name, const char *description, VerbObject &object)) {
			;
		}

		virtual auto has_verbs() -> bool { return false; }
		virtual auto print_verbs(const char *indent) -> bool { return false; }

		virtual bool print_summary(const char *indent, bool showContents = false) { return false; }

		virtual auto execute(Cli &cli, const char *verb, const char *parameters) -> bool { return false; }

		virtual Box<VerbObject> get_child(const char *path) { return *path?nullptr:clone(); }
	};

	struct DeviceObject: VerbObject {
		Driver &driver;

		/**/ DeviceObject(Driver &driver):
			driver(driver)
		{}
		Box<VerbObject> clone() override { return new DeviceObject(driver); }

		void get_children(void(*callback)(const char *name, const char *description, VerbObject &object)) override {
			;
		}

		auto has_verbs() -> bool override { return true; }
		auto print_verbs(const char *indent) -> bool override {
			if(driver.api.is_active()){
				log.print_info(indent, "This device is already active");
			}else{
				log.print_info(indent, format_verb, "activate", format_none, " - Activate this device");
			}

			if(driver.api.is_automatic()){
				log.print_info(indent, "This device starts automatically");
			}else if(!driver.api.is_disabled()){
				log.print_info(indent, "This device starts only when requested");
			}else{
				log.print_info(indent, "This device is disabled and will not be automatically started");
			}

			if(driver.api.is_active()){
				log.print_info(indent, "This device is active");
			}else{
				log.print_info(indent, format_verb, "start", format_none, " - Start this device");
			}

			if(driver.api.is_active()&&driver.can_stop_driver()){
				log.print_info(indent, format_verb, "stop", format_none, " - Stop this device");
			}else{
				log.print_info(indent, "This device cannot be stopped");
			}

			if(driver.api.is_active()&&driver.can_restart_driver()){
				log.print_info(indent, format_verb, "restart", format_none, " - Restart this device");
			}else{
				log.print_info(indent, "This device cannot be restarted");
			}

			if(driver.is_type<driver::Processor>()){
				log.print_info(indent, format_verb, "set . ", format_none, format_param, "<CLOCK>", format_none, format_verb, " <SPEED>", format_none, " - Set clock speed");

			}else if(driver.is_type<driver::Graphics>()){
				log.print_info(indent, format_verb, "mode . ", format_none, format_param, "<FRAMEBUFFER>", format_none, format_verb, " <WIDTH> <HEIGHT> <FORMAT> [exact|nearest]", format_none, " - Set framebuffer display mode");

			}else if(driver.is_type<driver::Serial>()){
				if(!driver.api.is_active()){
					log.print_info(indent, "This device is not currently active and cannot be sent to");
				}else{
					log.print_info(indent, format_verb, "send . ", format_none, format_verb, "<DATA>", format_none, " - Send/write data to this serial device");
				}
			}

			return true;
		}

		auto print_summary(const char *indent, bool showContents) -> bool override {
			drivers::print_driver_summary(indent, driver);
			if(showContents){
				log.print_info("");
				drivers::print_driver_details(indent, driver, format_param, format_none);
			}
			return true;
		}

		auto execute(Cli &cli, const char *verb, const char *parameters) -> bool override {
			if(!strcmp(verb, "start")){
				if(!drivers::start_driver(driver)){
					log.print_warning("This device driver cannot be started");
					return true;
				}

				log.print_info("Device started");
				return true;

			}else if(!strcmp(verb, "stop")){
				if(!drivers::stop_driver(driver)){
					log.print_warning("This device driver cannot be stopped");
					return true;
				}

				log.print_info("Device stopped");
				return true;

			}else if(!strcmp(verb, "restart")){
				if(!drivers::restart_driver(driver)){
					log.print_warning("This device driver cannot be restarted");
					return true;
				}

				log.print_info("Device restarted");
				return true;

			}else{
				if(driver.is_type<driver::Processor>()){
					// auto &processor = *(driver::Processor*)&driver;
					
					if(!strcmp(verb, "set")){
						//read clock name
						//read speed
						log.print_warning("TODO:Implement");
						return true;
					}

				}else if(driver.is_type<driver::Graphics>()){
					// auto &graphics = *(driver::Graphics*)&driver;
					
					if(!strcmp(verb, "mode")){
						//read framebuffer name
						//read width
						//read height
						//read format
						//read exact|nearest
						log.print_warning("TODO:Implement");
						return true;
					}

				}else if(driver.is_type<driver::Serial>()){
					auto &serial = *(driver::Serial*)&driver;

					if(!strcmp(verb, "send")){
						if(!serial.api.is_active()){
							log.print_warning("Cannot send, this serial device is not currently active");
							return true;
						}

						const auto data = parameters;
						serial.puts(data);

						return true;
					}
				}
			}

			return false;
		}
	};

	struct DevicesObject: VerbObject {
		Box<VerbObject> clone() override { return new DevicesObject; }

		void get_children(void(*callback)(const char *name, const char *description, VerbObject &object)) override {
			U32 i=0;
			for(auto &driver:drivers::iterate<driver::Processor>()){
				DeviceObject device(driver);
				char name[] = "cpu\0\0\0";
				callback(strcat(name, to_string(i++)), nullptr, device);
			}

			i=0;
			for(auto &driver:drivers::iterate<driver::Graphics>()){
				DeviceObject device(driver);
				char name[] = "graphics\0\0\0";
				callback(strcat(name, to_string(i++)), nullptr, device);
			}

			i=0;
			for(auto &driver:drivers::iterate<driver::Interrupt>()){
				DeviceObject device(driver);
				char name[] = "serial\0\0\0";
				callback(strcat(name, to_string(i++)), nullptr, device);
			}

			i=0;
			for(auto &driver:drivers::iterate<driver::Interrupt>()){
				DeviceObject device(driver);
				char name[] = "interrupt\0\0\0";
				callback(strcat(name, to_string(i++)), nullptr, device);
			}
		}

		Box<VerbObject> get_child(const char *path) override {
			U32 length=0;
			while(path[length]&&path[length]!='/') length++;

			const char *nextPath = path+length+(path[length]=='/'?1:0);

			if(!length) return *nextPath?get_child(nextPath):clone();

			U32 i=0;
			for(auto &driver:drivers::iterate<driver::Processor>()){
				DeviceObject device(driver);
				char name[] = "cpu\0\0\0";
				strcat(name, to_string(i++));
				if(strlen(name)==length&&!memcmp(name, path, length)){
					return DeviceObject(driver).get_child(nextPath);
				}
			}

			i=0;
			for(auto &driver:drivers::iterate<driver::Graphics>()){
				DeviceObject device(driver);
				char name[] = "graphics\0\0\0";
				strcat(name, to_string(i++));
				if(strlen(name)==length&&!memcmp(name, path, length)){
					return DeviceObject(driver).get_child(nextPath);
				}
			}

			i=0;
			for(auto &driver:drivers::iterate<driver::Interrupt>()){
				DeviceObject device(driver);
				char name[] = "serial\0\0\0";
				strcat(name, to_string(i++));
				if(strlen(name)==length&&!memcmp(name, path, length)){
					return DeviceObject(driver).get_child(nextPath);
				}
			}

			i=0;
			for(auto &driver:drivers::iterate<driver::Interrupt>()){
				DeviceObject device(driver);
				char name[] = "interrupt\0\0\0";
				strcat(name, to_string(i++));
				if(strlen(name)==length&&!memcmp(name, path, length)){
					return DeviceObject(driver).get_child(nextPath);
				}
			}

			return nullptr;
		}

		bool print_summary(const char *indent, bool showContents) override {
			U32 count = 0;
			for(auto &_:drivers::iterate<Driver>()){
				(void)_;
				count++;
			}

			log.print_info(indent, count, ' ', count==1?"device":"devices", " present");

			return true;
		}
	};

	struct ProcessesObject: VerbObject {
		Box<VerbObject> clone() override { return new ProcessesObject; }

		void get_children(void(*callback)(const char *name, const char *description, VerbObject &object)) override {
			// callback()
		}

		bool print_summary(const char *indent, bool showContents) override {
			auto count = process::get_count();
			log.print_info(indent, count, ' ', count==1?"process":"processes", " active");

			return true;
		}
	};

	struct RootObject: VerbObject {
		Box<VerbObject> clone() override { return new RootObject; }

		void get_children(void(*callback)(const char *name, const char *description, VerbObject &object)) override {
			DevicesObject devices;
			ProcessesObject processes;

			callback("device", "All devices", devices);
			callback("process", "All active processes", processes);
		}

		Box<VerbObject> get_child(const char *path) override {
			U32 length=0;
			while(path[length]&&path[length]!='/') length++;

			const char *nextPath = path+length+(path[length]=='/'?1:0);

			if(!length) return *nextPath?get_child(nextPath):clone();

			if(length==6&&!memcmp("device", path, length)){
				return DevicesObject{}.get_child(nextPath);

			}else if(length==7&&!memcmp("process", path, length)){
				return ProcessesObject{}.get_child(nextPath);

			}else{
				return nullptr;
			}
		}
	};

	struct Verb {
		const char *verbShort;
		const char *verb;
		const char *description;
		void(*execute)(Cli &cli, VerbObject *object, const char *path, const char *parameters);
	};

	Verb verbs[4] = {
		{ "?", "help", "Show help",
			[](Cli &cli, VerbObject *object, const char *path, const char *parameters) {
				log.print_info("Use ", format_verb, "verbs", format_none, " to list all currently valid actions");
				log.print_info("");
				log.print_info("Actions take the form `verb [path [parameters]]`");
			}
		},
		{ "v", "verbs", "List all valid verbs",
			[](Cli &cli, VerbObject *object, const char *path, const char *parameters) {
				log.print_info("Global verbs:");

				for(U32 i=0;i<sizeof(verbs)/sizeof(verbs[0]);i++){
					auto &verb = verbs[i];
					log.print_info_start();
					if(verb.verb[0]){
						log.print_inline("  ", format_verb, verb.verbShort, format_none, " / ", format_verb, verb.verb, format_none, " - ", verb.description);
					}else{
						log.print_inline("  ", format_verb, format_verb, verb.verbShort, format_none, " - ", verb.description);
					}
					log.print_end();
				}

				if(object&&object->has_verbs()){
					log.print_info("");
					log.print_info("Local verbs:");
					if(!object->print_verbs("  ")){
						log.print_info("  None available here");
					}
				}
			}
		},
		{ "ls", "", "List contents of current path",
			[](Cli &cli, VerbObject *object, const char *path, const char *parameters) {
				if(!object){
					log.print_warning("Path not found: ", path);
					log.print_info();
					return;
				}

				if(object->print_summary("", true)){
					log.print_info();
				}

				object->get_children([](const char *name, const char *description, VerbObject &object) {
					log.print_info_start();
					log.print_inline(format_object, name, format_none);
					if(description){
						log.print_inline(" - ", description);
					}
					log.print_end();

					object.print_summary("  ");
				});
				
				if(object->has_verbs()){
					log.print_info("Local verbs:");
					if(!object->print_verbs("  ")){
						log.print_info("  None available here");
					}
				}
			}
		},
		{ "cd", "", "Change to specified path",
			[](Cli &cli, VerbObject *object, const char *path, const char *parameters) {
				auto child = RootObject{}.get_child(path);
				if(!child){
					log.print_warning("Path not found: ", path);
					log.print_info();
					return;
				}

				delete cli.currentPath;
				cli.currentPath = strcpy(new C8[strlen(path)], path);
			}
		}
	};
}

/**/ Cli:: Cli() {
	currentPath = nullptr;
}

/**/ Cli::~Cli() {
	delete currentPath;
}

void Cli::prompt() {
	log.print_info_start();
	{
		U32 pathLength = strlen(currentPath);
		log.print_inline(cli::format_cwd);
		if(pathLength<32){
			log.print_inline(currentPath);
		}else{
			log.print_inline("...", &currentPath[pathLength-(32-3)]);
		}
		log.print_inline("> ", cli::format_none);
	}
	C8 buffer[1024];
	log.print_inline(cli::format_input);
	console::gets(buffer, sizeof(buffer));
	log.print_inline(cli::format_none);
	log.print_end();

	execute(buffer);
}

void Cli::execute(char *command) {
	while(cli::is_whitespace(*command)) command++;

	if(!*command) return;

	const char *inputVerb = command;

	if(!inputVerb[0]) return;

	while(*command&&!cli::is_whitespace(*command)) command++;
	while(cli::is_whitespace(*command)) *command++ = '\0';

	const char *inputSubject = command;

	while(*command&&!cli::is_whitespace(*command)) command++;
	while(cli::is_whitespace(*command)) *command++ = '\0';

	const char *inputParameters = command;

	bool commandFound = false;

	auto inputPath = cli::resolve_path(currentPath, inputSubject);

	auto child = cli::RootObject{}.get_child(inputPath);

	for(U32 i=0;i<sizeof(cli::verbs)/sizeof(cli::verbs[0]);i++){
		auto &verb = cli::verbs[i];

		if(!strcmp(verb.verbShort, inputVerb)||!strcmp(verb.verb, inputVerb)){
			commandFound = true;
			verb.execute(*this, child.get(), inputPath, inputParameters);
			log.print_info();
			break;
		}
	}

	if(!commandFound){
		if(!child){
			log.print_warning("Path not found: ", inputPath.get());
			log.print_info();
			return;
		}

		if(child->execute(*this, inputVerb, inputParameters)){
			log.print_info();
		}else{
			log.print_warning("I don't know how to `", inputVerb, "` here");
			log.print_info();
		}
	}
}
