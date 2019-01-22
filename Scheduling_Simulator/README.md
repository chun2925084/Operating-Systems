# Scheduling_Simulator

## Usage:
* makefiles:
````
  make
````
* scheduling_simulator:
````
$ add TASK_NAME -t TIME_QUANTUM –p PRIORITY
$ remove PID
$ start
$ ps
````





## Description
* Shell mode
  * Implement 4 commands (must follow the formats in slide 6)
  * add: Add new task(s)
  * remove: Remove task(s)
  * ps: Show the information of all tasks (PID, task name, task state,
queueing time, priority and time quantum)
  * start: Start or continue simulation (switch to simulation mode)
* Simulation mode
  * Use ucontext and the related APIs to implement context switch
  * Implement the priority-based variable-time-quantum RR(round robin)
scheduling
  * receive a signal (SIGALRM) every 10 ms (in the
Simulation mode), then determine whether to reschedule or not
  * Ctrl + z should pause the simulation and switch to shell mode
  * Time counting should be stopped in the Shell mode
  * start should resume the simulation
  * continue simulation from where it pauses
* Task
  * A task is a function in ‘tasks.c’ (task_name = function name)
![image](https://github.com/chun2925084/Operating-Systems/blob/master/Scheduling_Simulator/shell_simul.PNG)
![image](https://github.com/chun2925084/Operating-Systems/blob/master/Scheduling_Simulator/Task_state.PNG)
