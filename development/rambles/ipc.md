I think I am gonna end up doing things a bit like BeOS actually:

For IPC handling I'm gonna have processes define a callback entrypoint for processing incoming IPC messages - so every time one process sends to another the kernel fires up a new thread in the target process, starting at that entrypoint, and passes it the message. This means processes responding to things will always do so in new threads.

Someone clicks a button, a new IPC thread fires in that process with the "clicked button" message

Firing up a new thread is crazy cheap and means I can jump the scheduler into that thread entrypoint immediately, allocating it the remainder of the calling processes scheduled time, to keep message handling snappy 

I don't need to worry about it wasting cycles on other things and polling for updates. The thread that handles JUST that task has been fired, and executes immediately

in some cases I am just expecting processes to plop that message into a threadsafe queue, and get to it in their own time, later, from their main thread.

But hopefully for responsive things this happens in the new thread and things stay snappy

As far as cost, each thread will need a stack (prob 1-2MB), but it's otherwise just a few variables to store CPU state. All the memory mapping data is linked to the process, so spinning up new threads within them is cheap.

Processes will prob have a pool of IPC thread stacks, so they can quickly reuse old ones on launch.

I might limit the max count too - no more than 8 IPC handling threads active at once. Any more can get queued up; Don't wanna choke services

Actually, it might make sense to use the same entrypoint for everything. Starting the main thread could be an IPC message too. That way I could later add more program entrypoints for other IPC startup functions

```cpp
// runtime: (with more system ipcs being supported over time)

int entrypoint(IpcId ipc, void *ipcData) {
	switch(ipc) {
		case IpcId::init:
			__libc_init_array();
			return 0;

		case IpcId::start_main:
			return main();

		case IpcId::custom ... IpcId::custom_max:
			return ipcMain(ipc, ipcData);

		default:
			return -1; //unsupported system ipc message
	}
}

//program source:

int main() {
	//regular program entrypoint
}

int ipcMain(IpcId ipc, void *ipcData) {
	//entrypoint for custom ipc messages
}
```
