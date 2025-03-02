#include <drivers/DisplayManager.hpp>
#include <drivers/Processor.hpp>
#include <drivers/Scheduler.hpp>

#include <kernel/Cli.hpp>
#include <kernel/drivers.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/info.hpp>
#include <kernel/Log.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>
#include <kernel/memory/PagedPool.hpp>
#include <kernel/panic.hpp>
#include <kernel/Process.hpp>
#include <kernel/test.hpp>
#include <kernel/time.hpp>
#include <kernel/Thread.hpp>
#include <kernel/time.hpp>
#include <kernel/utils/logWindow.hpp>

#include <common/debugUtils.hpp>
#include <common/graphics2d/font.hpp>
#include <common/stdlib.hpp>

static Log log("kernel");

using namespace maths;

namespace memory {
	extern LList<::memory::Page> freePages;
	extern memory::PagedPool<sizeof(size_t)> kernelHeap;
}

namespace arm {
	namespace thread {
		extern LList<Thread> activeThreads;
		extern LList<Thread> sleepingThreads;
		extern LList<Thread> pausedThreads;
		extern LList<Thread> freedThreads;
	}
}

namespace libc {
	void init();
}

namespace drivers {
	void print_summary();
}

U64 vramWrites = 0;
U64 ramWrites = 0;

static DriverReference<driver::DisplayManager> displayManager{nullptr, [](void*){}, nullptr};
static DriverReference<driver::Scheduler> scheduler{nullptr, [](void*){}, nullptr};

namespace kernel {
	constinit bool isSafemode = false;

	void _preInit();
	void _postInit();

	[[noreturn]] void run() {
		libc::init();
		memory::init();
		logging::init();

		{ auto section = log.section("init");
			time::init();

			_preInit();

			scheduler = drivers::find_and_activate<driver::Scheduler>();
		}

		// set cpu to default speed (some devices start at min)
		if(auto processor = drivers::find_and_activate<driver::Processor>()){
			processor->set_clock_value(0, processor->get_clock_default(0));
		}

		{ auto section = log.section("startup");

			utils::logWindow::install();

			// panic::init();

			{ // try to load all remaining automatic drivers
				for(auto &driver:drivers::iterate()) {
					if(!driver.api.is_active()&&driver.api.is_automatic()) {
						(void)drivers::start_driver(driver);
					}
				}
			}

			{ auto section = log.section("device summary");

				log.print_info("cpu arch: ", info::cpu_arch);
				log.print_info("device type: ", info::device_type);
				log.print_info("device model: ", info::device_model);
				log.print_info("device revision: ", info::device_revision);
				log.print_info("memory: ", (memory::totalMemory/1024+1023)/1024, "MB");

				// { auto section = log.section("displays:");

				// 	for(auto i=0u;i<framebuffer::get_framebuffer_count();i++){
				// 		auto &framebuffer = *framebuffer::get_framebuffer(i);

				// 		log.print_info(framebuffer.buffer.width, 'x', framebuffer.buffer.height, ", ", framebuffer.buffer.format);
				// 	}
				// }

				{ auto section = log.section("drivers:");

					drivers::print_summary();
				}
			}

			log.print_info("Startup complete.");
		}

		_postInit();

		// asm volatile("sti");
		// asm volatile(
		// 	"mov al, 0x20\n"
		// 	"out 0x64, al\n"
		// 	"in al, 0x60\n"
		// 	"or al, 0x03\n"
		// 	"mov ah, al\n"
		// 	"mov al, 0x60\n"
		// 	"out 0x64, al\n"
		// 	"mov al, ah\n"
		// 	"out 0x60, al\n"

		// 	"mov al, 0xAE\n"
		// 	"out 0x64, al\n"

		// 	"mov al, 0xA8\n"
		// 	"out 0x64, al\n"

		// 	"mov al, 0xFF\n"
		// 	"out 0x60, al\n"

		// 	"mov al, 0xD4\n"
		// 	"out 0x64, al\n"
		// 	"mov al, 0xFF\n"
		// 	"out 0x60, al\n"

		// 	"mov al, 0xAE\n"
		// 	"out 0x64, al\n"

		// 	"mov al, 0xA8\n"
		// 	"out 0x64, al\n"
		// );
		// asm volatile(
		// 	"mov al, 0x20\n"
		// 	"out 0x20, al\n"
		// );

		test::start_tasks();

		while(true){
		// // 	// arch::x86::ioPort::write8(0x20, 0x0b);
		// // 	// log.print_info("PIC ISR = ", format::Hex8{arch::x86::ioPort::read8(0x20)});

		// // 	// arch::x86::ioPort::write8(0x20, 0x0a);
		// // 	// log.print_info("PIC IRR = ", format::Hex8{arch::x86::ioPort::read8(0x20)});
		}

		// debug::halt();

		// utils::logWindow::hide();

		#ifdef MEMORY_CHECKS
			debug_llist(memory::kernelHeap.availableBlocks, "availableBlocks 1");
		#endif

		// thread::create_kernel_thread("ram test", []() {
		// 	#ifdef MEMORY_CHECKS
		// 		debug_llist(memory::kernelHeap.availableBlocks, "availableBlocks 3");
		// 	#endif

		// 	const auto testBufferSize = 640*480*2;
		// 	U8 *testBuffer = new U8[testBufferSize];

		// 	while(true){
		// 		for(U32 i=0;i<testBufferSize;i++){
		// 			testBuffer[0] = i*2;
		// 			ramWrites++;
		// 		}
		// 	}

		// 	while(true){
		// 		scheduler->get_current_thread()->sleep(1000);
		// 	}
		// });

		#ifdef MEMORY_CHECKS
			debug_llist(memory::kernelHeap.availableBlocks, "availableBlocks 2");
		#endif

		auto &log = scheduler->get_current_thread()->process.log;

		displayManager = drivers::find_and_activate<driver::DisplayManager>();

		if(displayManager){
			log.print_info("Putting on lightshow...");

			// log.print_error("Error: BEFORE THREAD");
			// thread::create_kernel_thread("drawing test", []() {
			// 	log.print_error("Error: IN THREAD");

			// 	auto &framebuffer = framebuffer::framebuffers[0];

			// 	log.print("create_view\n");

			// 	auto view = displayManager->create_view(100, 100, 640, 480);
			// 	if(!view) {
			// 		log.print("NO VIEW\n");
			// 		return;
			// 	}

			// 	int dirX = 3;
			// 	int dirY = 3;

			// 	log.print("start drawing\n");

			// 	for(auto time=0u;;time++){
			// 		const auto padding = 2u;
			// 		for(auto y=0u+padding;y<view->buffer.height-padding;y++)for(auto x=0u+padding;x<view->buffer.width-padding;x++) {
			// 			view->buffer.set(x, y, time<<24|((x+time)%256)<<16|((x-time-y)%256)<<8|((x+time+y)%256));
			// 			vramWrites++;
			// 		}

			// 		// displayManager->update_view(*view);

			// 		if(dirX<0&&view->x+dirX<-30||dirX>0&&view->x+view->buffer.width+dirX>=framebuffer.buffer.width+30) dirX *= -1;
			// 		if(dirY<0&&view->y+dirY<-30||dirY>0&&view->y+view->buffer.height+dirY>=framebuffer.buffer.height+30) dirY *= -1;

			// 		displayManager->move_view_to(*view, view->x+dirX, view->y+dirY);

			// 		// log.print("update view\n");
			// 	}
			// });

			auto process = process::create_kernel("lightshow");

			// for(auto i=0;i<2;i++)process.create_kernel_thread([]() {
			// 	auto &log = scheduler->get_current_thread()->process.log;

			// 	// const process = scheduler->get_current_thread()->load();
			// 	while(true){
			// 		log.print_info("thread 1");

			// 		timer::wait(100000);

			// 		log.print_info("thread 2");
			// 	}
			// });

			if(true){
				for(int i=0;i<4;i++){
					auto &thread = process.create_kernel_thread([]() {
						auto &log = scheduler->get_current_thread()->process.log;

						log.print_debug("entered thread!");

						auto width = 640u/(rand()%3+1);
						auto height = 480u/(rand()%3+1);
						auto scale = rand()%3+1;
						auto life = 300;

						log.print_debug("got view ", width, "x", height, " @ ", scale);

						width /= scale;
						height /= scale;

						(void) width;
						(void) height;

						auto view = displayManager->create_display(scheduler->get_current_thread(), driver::DisplayManager::DisplayLayer::regular, rand()%(displayManager->get_width()-width), rand()%(displayManager->get_height()-height), width, height, scale);
						if(!view) {
							log.print_error("Error: didn't get a view");
							return;
						}

						log.print_debug("got view");

						int dirX = rand()%2?+1:-1;
						int dirY = rand()%2?+1:-1;
						int timeDir = rand()%2?+1:-1;

						U32 pixelTime = 3000;

						auto lastTime = time::now();

						for(I32 time=0;;time+=3*timeDir){
							const auto padding = 2u;
							for(auto y=0u+padding+abs(time)%4;y<view->buffer.height-padding;y+=2)for(auto x=0u+padding;x<view->buffer.width-padding;x++) {
								// view->buffer.set(x, y, (time*4)<<24|((x+(time*4))%256)<<16|((x-(time*4)-y)%256)<<8|((x+(time*4)+y)%256));
								view->buffer.set(x, y, (time*4)<<24|((x+(time*4))%256)<<16|((x-(time*4)-y)%256)<<8|((x+(time*4)+y)%256));
								vramWrites++;
							}

							// displayManager->update_view(*view);

							auto now = time::now();
							U32 steps = (U32)(min((U64)30000u, now-lastTime))/pixelTime;
							lastTime += steps * pixelTime;

							I32 deltaX = dirX * (I32)steps;
							I32 deltaY = dirY * (I32)steps;
							// I32 deltaX = dirX * 3;
							// I32 deltaY = dirY * 3;

							if(deltaX<0&&view->x+deltaX<-30||deltaX>0&&view->x+view->get_width()+deltaX>=displayManager->get_width()+30){
								dirX *= -1;
								deltaX *= -1;
							}
							if(deltaY<0&&view->y+deltaY<-30||deltaY>0&&view->y+view->get_height()+deltaY>=displayManager->get_height()+30){
								dirY *= -1;
								deltaY *= -1;
							}

							if(deltaX||deltaY){
								// auto before = time::now();
								// displayManager->update_view(*view);
								view->move_to(view->x+deltaX, view->y+deltaY);
								// auto updateTime = time::now()-before;
								// log.print_debug(updateTime, " for ", width, "x", height, " @ ", scale, " \n");
							}else{
								// interrupts::Guard guard;
								view->update();
							}

							// log.print_debug("interrupts::_lock_depth: ", raspi::interrupts::_lock_depth.load(), "\n");

							// timer::wait(1000000);

							// scheduler->yield();

							life--;
							// if(life<1) break;
						}
					});

					scheduler->add_thread(thread);
				}

			}
			
			if(true){
				for(auto i=0;i<5;i++){
					auto &thread = process.create_kernel_thread([]() {
						float scale = 0.25+(rand()%256)/128.0;

						auto &thread = *scheduler->get_current_thread();
						auto &log = thread.process.log;

						I32 width = 985*scale;
						I32 height = 128*scale;

						log.print_info(width, "x", height);

						I32 x = rand()%(800+width);
						I32 y = rand()%500;
						I32 speed = rand()%6+1;

						auto view = displayManager->create_display(scheduler->get_current_thread(), driver::DisplayManager::DisplayLayer::regular, x, y, width, height, 1.0);

						auto &buffer = view->buffer;

						U8 bgColour = rand()%0x18;

						buffer.draw_rect(0, 0, width, height, bgColour<<16|bgColour<<8|bgColour);

						auto startTime = time::now();
						buffer.draw_text(*graphics2d::font::default_sans, "Lots of test text!", 10*scale+6, (128-20)*scale+6, 9999, 128*scale, 0x000000);
						buffer.draw_text(*graphics2d::font::default_sans, "Lots of test text!", 10*scale, (128-20)*scale, 9999, 128*scale, 0xdddddd);
						log.print_debug("blitted in ", time::now()-startTime);

						view->update();

						// float scale = 1.0;
						// float scaleDelta = 0.1;

						while(true){
							// const auto startTime = time::now();

							x -= speed*4;

							if(x+width<1){
								x = displayManager->get_width();
								y = rand()%550-50;
								// break;
							}

							view->move_to(x, y);
							// scheduler->yield();
							// thread.sleep(1000000/60-(time::now()-startTime));

							// buffer.draw_rect(0, 0, width, height, 0x111111);
							// buffer.draw_msdf(10-(scale-1)*graphics2d::font::openSans.atlas.width/2, 10-(scale-1)*graphics2d::font::openSans.atlas.height/2, graphics2d::font::openSans.atlas.width*scale, graphics2d::font::openSans.atlas.height*scale, graphics2d::font::openSans.atlas, 0, 0, graphics2d::font::openSans.atlas.width, graphics2d::font::openSans.atlas.height, 0xffffff);
							// displayManager->update_view(*view);
							// scheduler->yield();
							// if(scale+scaleDelta>30){
							// 	scaleDelta = -abs(scaleDelta);
							// }else if(scale+scaleDelta<1){
							// 	scaleDelta = +abs(scaleDelta);
							// }
							// scale *= 1.0+scaleDelta;
						}
					});

					scheduler->add_thread(thread);
				}
			}

			// process.create_kernel_thread([]() {
			// 	Cli cli;

			// 	while(true){
			// 		cli.prompt();
			// 	}
			// });

			// while(true){
			// 	scheduler->get_current_thread()->sleep(1000000);
			// 	{
			// 		auto section = log.section("Status:");

			// 		log.print_debug("Active threads: ", scheduler::get_active_thread_count()-1 /* exclude self since we were sleeping throughout */, '/', scheduler::get_total_thread_count(), "\n");
			// 		log.print_debug("VRAM: ", vramWrites, " writes/sec\n");
			// 		log.print_debug(" RAM: ", ramWrites, " writes/sec\n");
			// 	}
			// 	vramWrites = 0;
			// 	ramWrites = 0;
			// }
		}

		// finally put this thread to sleep, and keep it that way, should it ever be re-awakened
		while(true){
			// scheduler->get_current_thread()->pause();
			scheduler->yield();
		}
	}
}
