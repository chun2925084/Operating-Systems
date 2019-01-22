# Simple-pstree

## How to execute
simple_pstree [-c|-s|-p][pid]
* -c：Display the entire process tree which is spawned by a process
  * The tree of processes is rooted at either pid or init if pid is omitted
  * This is the default option
* -s：Display all siblings of a process
  * The searching PID of the process can be either pid or simple_pstree if pid is
omitted
* -p：Display all ancestors of a process
  * The searching PID of the process can be either pid or simple_pstree if pid is
omitted
