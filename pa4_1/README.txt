Question 1. Why do we block SIGUSR2 and SIGALRM in worker.c? Why do we unblock SIGUSR1
and SIGTERM in worker.c?
    >> signals are inhreited by the threads. SIGUSR2 and SIGALRM are not intended to be caught by  the worker. We unblock SIGUSR1 and SIGTERM for the same reason. 


Question 2. We use sigwait() and sigaction() in our code. Explain the difference between the two. (Please explain from the aspect of thread behavior rather than syntax).
    >> sigaction() - is used to set up a signal handler. Any signal has a default handler, but if we need to enact a different set of instructions instead of default, we can do that by using sigaction. we can see an example of this in setup_sig_handlers() function. 
    >> sigwait() - is required to wait for a signal. In the conntext of the program, when the thread is suspended, it needs to wait for a signal to resume. With sigwait, we can block the thread until the specified signal occurs. 


Question 3. When we use POSIX:TMR interval timer, we are using relative time. What is the
alternative? Explain the difference between the two.
    >> The alternative is to use absolute time. In case of absolute time, we can set the time on the timer to issue a signal at a specific time. .i.e. we can setup timer to issue a signal at certain date and time eg. 15th of may 2016, 12:00:00. In this case, we need to calcuate the time for alarm signal in absolute time. .ie. 15th of may 2016 12:00:01. When this is fired, we need to calcuate and set for next alarm 15th of may 2016 12:00:02 etc
    >> In relative time however, we are refering to time in a relative way. .i.e. to say, issue an alarm 2 seconds from now. 


Question 4. Look at start_worker() in worker.c, a worker thread is executing within an infinite loop at the end. When does a worker thread terminate?
    >> The thread is terminated by the sigterm signal. Whenever we issue the signal, the program switches over to signal handler, cancel_thread() where we are exiting thread. 


Question 5. When does the scheduler finish? Why does it not exit when the scheduler queue is empty?
    >> The scheduler finishes when all the worker threads are dead. 
    The scheduler queue only manages the threads that are currently selected for execution. There is a limit on the number of workers in scheduler queue. thus when the queue is empty, we need to check if there is any worker waiting to get into scheduler queue. 
    Initially the scheduler queue is empty, even though worker threads are created. Thus scheduler being empty is not the right condition to exit scheduler. 

Question 6. After a thread is scheduled to run, is it still in the sched_queue? When is it removed from the head of the queue? When is it removed from the queue completely?
    >> The thread stays in the scheduler queue for as long as it is not complete. When the thread is scheduled, we keep the thread in the head of the queue. When we suspend the thread (after the quantum expires), we move the thread to the back of the queue. 
    We remove the thread completely when the thread has completed executing (when runtime == requested run time). We then kill the thread and remove from queue. 


Question 7. What's the purpose of the global variable "completed" in scheduler.c? Why do we
compare "completed" with thread_count before we wait_for_queue() in next_worker()?
    >> The purpose of completed is to keep track of the number of workers that have completed execution. We update this when a thread terminates. 
    if the number of completed thead (completed) is less than the thread_count, then this indicates that there is a thread that is still to execute. 


Question 8. We only implemented Round Robin in this PA. If we want to implement a FIFO
scheduling algorithm and keep the modification as minimum, which function in scheduler.c is the one that you should modify? Briefly describe how you would modify this function.
    >> if we need to keep the modifications to the minimum, we need to modify suspend_worker() function to implement fifo. In suspend_worker, we are moving the worker to the back of the queue upon execution. Instead, we we comment out this part of the code, the worker remains in the head of the queue till it is removed by cancel_worker, and thus scheduled for execution by next_worker() as it only gets worker at head position. 


Question 9. In this implementation, the scheduler only changes threads when the time quantum
expires. Briefly explain how you would use an additional signal to allow the scheduler to change threads in the middle of a time quantum. In what situations would this be useful?
    >> In some cases, it is possible for the worker to complete execution the end of quantum (the thread could terminate or block for io). In this case, we can inform scheduler to schedule a new thread instead of wasting time on cpu. 
    >> When the thread wants to block, we can use a new signal (from the current thread to scheduler), to inform of its intention. A new signal handler needs to be setup on scheduler, to reset the timer (if needed) and schedule new thread. 
