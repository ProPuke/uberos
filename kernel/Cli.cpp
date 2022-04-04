#include "Cli.hpp"

#include <common/Box.hpp>

#include <kernel/device.hpp>
#include <kernel/Driver.hpp>
#include <kernel/driver/Processor.hpp>
#include <kernel/driver/Interrupt.hpp>
#include <kernel/driver/Graphics.hpp>
#include <kernel/driver/Serial.hpp>
#include <kernel/memory.hpp>
#include <kernel/mmio.hpp>
#include <kernel/Process.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/stdio.hpp>

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
			switch(driver.state){
				case Driver::State::enabled:
					stdio::print_info(indent, "This device is already enabled");
				break;
				case Driver::State::restarting:
					stdio::print_info(indent, "This device is restarting");
				break;
				default:
					stdio::print_info(indent, format_verb, "enable", format_none, " - Enable this device");
			}
			if(driver.can_disable_driver()){
				stdio::print_info(indent, format_verb, "disable", format_none, " - Disable this device");
			}else{
				stdio::print_info(indent, "This device cannot be disabled");
			}
			if(driver.can_restart_driver()){
				stdio::print_info(indent, format_verb, "restart", format_none, " - Restart this device");
			}else{
				stdio::print_info(indent, "This device cannot be restarted");
			}

			if(!strcmp(driver.type, "processor")){
				stdio::print_info(indent, format_verb, "set . ", format_none, format_param, "<CLOCK>", format_none, format_verb, " <SPEED>", format_none, " - Set clock speed");

			}else if(!strcmp(driver.type, "graphics")){
				stdio::print_info(indent, format_verb, "mode . ", format_none, format_param, "<FRAMEBUFFER>", format_none, format_verb, " <WIDTH> <HEIGHT> <FORMAT> [exact|nearest]", format_none, " - Set framebuffer display mode");

			}else if(!strcmp(driver.type, "serial")){
				if(driver.state!=Driver::State::enabled){
					stdio::print_info(indent, "This device is not currently enabled and cannot be sent to");
				}else{
					stdio::print_info(indent, format_verb, "send . ", format_none, format_verb, "<DATA>", format_none, " - Send/write data to this serial device");
				}
			}

			return true;
		}

		auto print_summary(const char *indent, bool showContents) -> bool override {
			device::print_device_summary(indent, driver);
			if(showContents){
				stdio::print_info("");
				device::print_device_details(indent, driver, format_param, format_none);
			}
			return true;
		}

		auto execute(Cli &cli, const char *verb, const char *parameters) -> bool override {
			if(!strcmp(verb, "enable")){
				if(!device::start_device(driver)){
					stdio::print_warning("This device driver cannot be enabled");
					return true;
				}

				stdio::print_info("Device enabled");
				return true;

			}else if(!strcmp(verb, "disable")){
				if(!device::stop_device(driver)){
					stdio::print_warning("This device driver cannot be disabled");
					return true;
				}

				stdio::print_info("Device disabled");
				return true;

			}else if(!strcmp(verb, "restart")){
				if(!device::restart_device(driver)){
					stdio::print_warning("This device driver cannot be restarted");
					return true;
				}

				stdio::print_info("Device restarted");
				return true;

			}else{
				if(!strcmp(driver.type, "processor")){
					// auto &processor = *(driver::Processor*)&driver;
					
					if(!strcmp(verb, "set")){
						//read clock name
						//read speed
						stdio::print_warning("TODO:Implement");
						return true;
					}

				}else if(!strcmp(driver.type, "graphics")){
					// auto &graphics = *(driver::Graphics*)&driver;
					
					if(!strcmp(verb, "mode")){
						//read framebuffer name
						//read width
						//read height
						//read format
						//read exact|nearest
						stdio::print_warning("TODO:Implement");
						return true;
					}

				}else if(!strcmp(driver.type, "serial")){
					auto &serial = *(driver::Serial*)&driver;

					if(!strcmp(verb, "send")){
						if(serial.state!=Driver::State::enabled){
							stdio::print_warning("Cannot send, this serial device is not currently enabled");
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
			for(auto &driver:device::iterate_type<driver::Processor>("processor")){
				DeviceObject device(driver);
				char name[] = "cpu\0\0\0";
				callback(strcat(name, to_string(i++)), nullptr, device);
			}

			i=0;
			for(auto &driver:device::iterate_type<driver::Graphics>("graphics")){
				DeviceObject device(driver);
				char name[] = "graphics\0\0\0";
				callback(strcat(name, to_string(i++)), nullptr, device);
			}

			i=0;
			for(auto &driver:device::iterate_type<driver::Interrupt>("serial")){
				DeviceObject device(driver);
				char name[] = "serial\0\0\0";
				callback(strcat(name, to_string(i++)), nullptr, device);
			}

			i=0;
			for(auto &driver:device::iterate_type<driver::Interrupt>("interrupt")){
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
			for(auto &driver:device::iterate_type<driver::Processor>("processor")){
				DeviceObject device(driver);
				char name[] = "cpu\0\0\0";
				strcat(name, to_string(i++));
				if(strlen(name)==length&&!memcmp(name, path, length)){
					return DeviceObject(driver).get_child(nextPath);
				}
			}

			i=0;
			for(auto &driver:device::iterate_type<driver::Graphics>("graphics")){
				DeviceObject device(driver);
				char name[] = "graphics\0\0\0";
				strcat(name, to_string(i++));
				if(strlen(name)==length&&!memcmp(name, path, length)){
					return DeviceObject(driver).get_child(nextPath);
				}
			}

			i=0;
			for(auto &driver:device::iterate_type<driver::Interrupt>("serial")){
				DeviceObject device(driver);
				char name[] = "serial\0\0\0";
				strcat(name, to_string(i++));
				if(strlen(name)==length&&!memcmp(name, path, length)){
					return DeviceObject(driver).get_child(nextPath);
				}
			}

			i=0;
			for(auto &driver:device::iterate_type<driver::Interrupt>("interrupt")){
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
			for(auto &_:device::iterate_all()){
				(void)_;
				count++;
			}

			stdio::print_info(indent, count, ' ', count==1?"device":"devices", " present");

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
			stdio::print_info(indent, count, ' ', count==1?"process":"processes", " active");

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
				stdio::print_info("Use ", format_verb, "verbs", format_none, " to list all currently valid actions");
				stdio::print_info("");
				stdio::print_info("Actions take the form `verb [path [parameters]]`");
			}
		},
		{ "v", "verbs", "List all valid verbs",
			[](Cli &cli, VerbObject *object, const char *path, const char *parameters) {
				stdio::print_info("Global verbs:");

				for(U32 i=0;i<sizeof(verbs)/sizeof(verbs[0]);i++){
					auto &verb = verbs[i];
					stdio::print_info_start();
					if(verb.verb[0]){
						stdio::print_inline("  ", format_verb, verb.verbShort, format_none, " / ", format_verb, verb.verb, format_none, " - ", verb.description);
					}else{
						stdio::print_inline("  ", format_verb, format_verb, verb.verbShort, format_none, " - ", verb.description);
					}
					stdio::print_end();
				}

				if(object&&object->has_verbs()){
					stdio::print_info("");
					stdio::print_info("Local verbs:");
					if(!object->print_verbs("  ")){
						stdio::print_info("  None available here");
					}
				}
			}
		},
		{ "ls", "", "List contents of current path",
			[](Cli &cli, VerbObject *object, const char *path, const char *parameters) {
				if(!object){
					stdio::print_warning("Path not found: ", path);
					stdio::print_info();
					return;
				}

				if(object->print_summary("", true)){
					stdio::print_info();
				}

				object->get_children([](const char *name, const char *description, VerbObject &object) {
					stdio::print_info_start();
					stdio::print_inline(format_object, name, format_none);
					if(description){
						stdio::print_inline(" - ", description);
					}
					stdio::print_end();

					object.print_summary("  ");
				});
				
				if(object->has_verbs()){
					stdio::print_info("Local verbs:");
					if(!object->print_verbs("  ")){
						stdio::print_info("  None available here");
					}
				}
			}
		},
		{ "cd", "", "Change to specified path",
			[](Cli &cli, VerbObject *object, const char *path, const char *parameters) {
				auto child = RootObject{}.get_child(path);
				if(!child){
					stdio::print_warning("Path not found: ", path);
					stdio::print_info();
					return;
				}

				delete cli.currentPath;
				cli.currentPath = strcpy(new C8[strlen(path)], path);
			}
		}
	};
}

/**/ Cli:: Cli() {
	currentPath = new char[1];
	currentPath[0] = '\0';
}

/**/ Cli::~Cli() {
	delete currentPath;
}

void Cli::prompt() {
	stdio::print_info_start();
	{
		U32 pathLength = strlen(currentPath);
		stdio::print_inline(cli::format_cwd);
		if(pathLength<32){
			stdio::print_inline(currentPath);
		}else{
			stdio::print_inline("...", &currentPath[pathLength-(32-3)]);
		}
		stdio::print_inline("> ", cli::format_none);
	}
	C8 buffer[1024];
	stdio::print_inline(cli::format_input);
	stdio::gets(buffer, sizeof(buffer));
	stdio::print_inline(cli::format_none);
	stdio::print_end();

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
			stdio::print_info();
			break;
		}
	}

	if(!commandFound){
		if(!child){
			stdio::print_warning("Path not found: ", inputPath.get());
			stdio::print_info();
			return;
		}

		if(child->execute(*this, inputVerb, inputParameters)){
			stdio::print_info();
		}else{
			stdio::print_warning("I don't know how to `", inputVerb, "` here");
			stdio::print_info();
		}
	}
}
