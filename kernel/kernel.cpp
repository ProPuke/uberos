#include <common/debugUtils.hpp>
#include <common/graphics2d/font.hpp>
#include <common/stdlib.hpp>

#include <kernel/arch/raspi/kernel.hpp>
#include <kernel/arch/raspi/timer.hpp>
#include <kernel/Cli.hpp>
#include <kernel/cpu.hpp>
#include <kernel/device.hpp>
#include <kernel/exceptions.hpp>
#include <kernel/framebuffer.hpp>
#include <kernel/graphics2d.hpp>
#include <kernel/info.hpp>
#include <kernel/memory.hpp>
#include <kernel/memory/PagedPool.hpp>
#include <kernel/Process.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/Spinlock.hpp>
#include <kernel/log.hpp>
#include <kernel/Thread.hpp>
#include <kernel/timer.hpp>
#include <kernel/utils/logWindow.hpp>

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

namespace exceptions {
	void init();
}

namespace scheduler {
	void init();
}

namespace device {
	void print_summary();
}

U64 vramWrites = 0;
U64 ramWrites = 0;

namespace kernel {
	void init(void(*preinit)(), void(*init)(), void(*postinit)()) {
		if(preinit) preinit();

		libc::init();

		cpu::init();

		{ log::Section section("kernel init");
			exceptions::init();
	
			if(init) init();

			scheduler::init();
		}

		{ log::Section section("kernel startup");

			graphics2d::init();

			utils::logWindow::install();

			{ log::Section section("device summary");

				log::print_info("cpu arch: ", info::cpu_arch);
				log::print_info("device type: ", info::device_type);
				log::print_info("device model: ", info::device_model);
				log::print_info("device revision: ", info::device_revision);
				log::print_info("memory: ", memory::totalMemory/1024/1024, "MB");

				{ log::Section section("displays:");

					for(auto i=0u;i<framebuffer::get_framebuffer_count();i++){
						auto &framebuffer = *framebuffer::get_framebuffer(i);

						log::print_info(framebuffer.buffer.width, 'x', framebuffer.buffer.height, ", ", framebuffer.buffer.format);
					}
				}

				{ log::Section section("devices:");

					device::print_summary();
				}
			}

			log::print_info("Startup complete.");
		}

		if(postinit) postinit();

		utils::logWindow::hide();

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
		// 		thread::currentThread->sleep(1000);
		// 	}
		// });

		#ifdef MEMORY_CHECKS
			debug_llist(memory::kernelHeap.availableBlocks, "availableBlocks 2");
		#endif

		auto &log = thread::currentThread.load()->process.log;

		log.print_info("Putting on lightshow...");

		// log::print_error("Error: BEFORE THREAD");
		// thread::create_kernel_thread("drawing test", []() {
		// 	log::print_error("Error: IN THREAD");

		// 	auto &framebuffer = framebuffer::framebuffers[0];

		// 	log::print("create_view\n");

		// 	auto view = graphics2d::create_view(100, 100, 640, 480);
		// 	if(!view) {
		// 		log::print("NO VIEW\n");
		// 		return;
		// 	}

		// 	int dirX = 3;
		// 	int dirY = 3;

		// 	log::print("start drawing\n");

		// 	for(auto time=0u;;time++){
		// 		const auto padding = 2u;
		// 		for(auto y=0u+padding;y<view->buffer.height-padding;y++)for(auto x=0u+padding;x<view->buffer.width-padding;x++) {
		// 			view->buffer.set(x, y, time<<24|((x+time)%256)<<16|((x-time-y)%256)<<8|((x+time+y)%256));
		// 			vramWrites++;
		// 		}

		// 		// graphics2d::update_view(*view);

		// 		if(dirX<0&&view->x+dirX<-30||dirX>0&&view->x+view->buffer.width+dirX>=framebuffer.buffer.width+30) dirX *= -1;
		// 		if(dirY<0&&view->y+dirY<-30||dirY>0&&view->y+view->buffer.height+dirY>=framebuffer.buffer.height+30) dirY *= -1;

		// 		graphics2d::move_view_to(*view, view->x+dirX, view->y+dirY);

		// 		// log::print("update view\n");
		// 	}
		// });

		auto process = process::create_kernel("lightshow", [](){
			while(true) {
				thread::currentThread.load()->sleep(1000);
			}
		});

		// for(auto i=0;i<2;i++)process.create_kernel_thread([]() {
		// 	auto &log = thread::currentThread.load()->process.log;

		// 	// const process = thread::currentThread->load();
		// 	while(true){
		// 		log.print_info("thread 1");

		// 		timer::udelay(100000);

		// 		log.print_info("thread 2");
		// 	}
		// });

		if(true){
			for(int i=0;i<4;i++){
				process.create_kernel_thread([]() -> I32 {
					auto &log = thread::currentThread.load()->process.log;
					auto possibleFramebuffer = framebuffer::get_framebuffer(0);
					if(!possibleFramebuffer){
						log.print_error("No valid framebuffer");
						return 0;
					}

					auto &framebuffer = *possibleFramebuffer;

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
					(void) framebuffer;

					auto view = graphics2d::create_view(thread::currentThread, graphics2d::ViewLayer::regular, rand()%((U32)framebuffer.buffer.width-width), rand()%((U32)framebuffer.buffer.height-height), width, height, scale);
					if(!view) {
						log.print_error("Error: didn't get a view");
						return 0;
					}

					log.print_debug("got view");

					int dirX = rand()%2?+1:-1;
					int dirY = rand()%2?+1:-1;
					int timeDir = rand()%2?+1:-1;

					U32 pixelTime = 3000;

					auto lastTime = timer::now();

					for(I32 time=0;;time+=3*timeDir){
						const auto padding = 2u;
						for(auto y=0u+padding+abs(time)%4;y<view->buffer.height-padding;y+=2)for(auto x=0u+padding;x<view->buffer.width-padding;x++) {
							// view->buffer.set(x, y, (time*4)<<24|((x+(time*4))%256)<<16|((x-(time*4)-y)%256)<<8|((x+(time*4)+y)%256));
							view->buffer.set(x, y, (time*4)<<24|((x+(time*4))%256)<<16|((x-(time*4)-y)%256)<<8|((x+(time*4)+y)%256));
							vramWrites++;
						}

						// graphics2d::update_view(*view);

						auto now = timer::now();
						U32 steps = (U32)(min(30000u, now-lastTime))/pixelTime;
						lastTime += steps * pixelTime;

						I32 deltaX = dirX * (I32)steps;

						I32 deltaY = dirY * (I32)steps;
						// I32 deltaX = dirX * 3;
						// I32 deltaY = dirY * 3;

						if(deltaX<0&&view->x+deltaX<-30||deltaX>0&&view->x+view->buffer.width*view->scale+deltaX>=framebuffer.buffer.width+30){
							dirX *= -1;
							deltaX *= -1;
						}
						if(deltaY<0&&view->y+deltaY<-30||deltaY>0&&view->y+view->buffer.height*view->scale+deltaY>=framebuffer.buffer.height+30){
							dirY *= -1;
							deltaY *= -1;
						}

						if(deltaX||deltaY){
							scheduler::Guard guard;
							// auto before = timer::now();
							// graphics2d::update_view(*view);
							graphics2d::move_view_to(*view, view->x+deltaX, view->y+deltaY);
							// auto updateTime = timer::now()-before;
							// log.print_debug(updateTime, " for ", width, "x", height, " @ ", scale, " \n");
						}else{
							// interrupts::Guard guard;
							graphics2d::update_view(*view);
						}

						// log.print_debug("interrupts::_lock_depth: ", raspi::interrupts::_lock_depth.load(), "\n");

						// timer::udelay(1000000);

						scheduler::yield();

						life--;
						// if(life<1) break;
					}
				});
			}

		}
		
		if(true){
			for(auto i=0;i<5;i++)process.create_kernel_thread([]() -> I32 {
				float scale = 0.25+(rand()%256)/128.0;

				auto &thread = *::thread::currentThread.load();
				auto &log = thread.process.log;

				I32 width = 985*scale;
				I32 height = 128*scale;

				log.print_info(width, "x", height);

				I32 x = rand()%(800+width);
				I32 y = rand()%500;
				I32 speed = rand()%6+1;

				auto view = graphics2d::create_view(thread::currentThread, graphics2d::ViewLayer::regular, x, y, width, height, 1.0);

				auto &buffer = view->buffer;

				U8 bgColour = rand()%0x18;

				buffer.draw_rect(0, 0, width, height, bgColour<<16|bgColour<<8|bgColour);

				auto startTime = timer::now();
				buffer.draw_text(*graphics2d::font::default_sans, "Lots of test text!", 10*scale+6, (128-20)*scale+6, 128*scale, 0x000000);
				buffer.draw_text(*graphics2d::font::default_sans, "Lots of test text!", 10*scale, (128-20)*scale, 128*scale, 0xdddddd);
				log.print_debug("blitted in ", timer::now()-startTime);

				auto possibleFramebuffer = framebuffer::get_framebuffer(0);
				if(!possibleFramebuffer){
					log.print_error("No valid framebuffer");
					return 0;
				}

				auto &framebuffer = *possibleFramebuffer;

				graphics2d::update_view(*view);

				// float scale = 1.0;
				// float scaleDelta = 0.1;

				while(true){
					// const auto startTime = timer::now();

					x -= speed*4;

					if(x+width<1){
						x = framebuffer.buffer.width;
						y = rand()%550-50;
						// break;
					}

					graphics2d::move_view_to(*view, x, y);
					scheduler::yield();
					// thread.sleep(1000000/60-(timer::now()-startTime));

					// buffer.draw_rect(0, 0, width, height, 0x111111);
					// buffer.draw_msdf(10-(scale-1)*graphics2d::font::openSans.atlas.width/2, 10-(scale-1)*graphics2d::font::openSans.atlas.height/2, graphics2d::font::openSans.atlas.width*scale, graphics2d::font::openSans.atlas.height*scale, graphics2d::font::openSans.atlas, 0, 0, graphics2d::font::openSans.atlas.width, graphics2d::font::openSans.atlas.height, 0xffffff);
					// graphics2d::update_view(*view);
					// scheduler::yield();
					// if(scale+scaleDelta>30){
					// 	scaleDelta = -abs(scaleDelta);
					// }else if(scale+scaleDelta<1){
					// 	scaleDelta = +abs(scaleDelta);
					// }
					// scale *= 1.0+scaleDelta;
				}
			});
		}

		// process.create_kernel_thread([]() -> I32 {
		// 	Cli cli;

		// 	while(true){
		// 		cli.prompt();
		// 	}
		// });

		// while(true){
		// 	thread::currentThread.load()->sleep(1000000);
		// 	{
		// 		log::Section section("Status:");

		// 		log::print_debug("Active threads: ", scheduler::get_active_thread_count()-1 /* exclude self since we were sleeping throughout */, '/', scheduler::get_total_thread_count(), "\n");
		// 		log::print_debug("VRAM: ", vramWrites, " writes/sec\n");
		// 		log::print_debug(" RAM: ", ramWrites, " writes/sec\n");
		// 	}
		// 	vramWrites = 0;
		// 	ramWrites = 0;
		// }

		// finally put this thread to sleep, and keep it that way, should it ever be re-awakened
		while(true){
			thread::currentThread.load()->pause();
			scheduler::yield();
		}
	}
}
