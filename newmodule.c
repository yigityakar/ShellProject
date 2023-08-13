#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/fcntl.h>

int root_pid;
module_param(root_pid, int, 0);
MODULE_PARM_DESC(root_pid, "This is the root PID of the subprocess tree.");
int heir = 1; // heir is used as a flag to get the first child

int simple_init(void){

    struct task_struct *root_task;
    struct task_struct *task;
    struct list_head *list;

    printk(KERN_WARNING "Starting kernel module\n");
    printk("Digraph {");

    /* find the process with the root pid */
    for_each_process(task){
        if (task->pid == root_pid){
            root_task = task;
            break;
        }   
    }
    if (root_task == NULL){
        printk(KERN_WARNING "Process with PID: %d cannot be found\n", root_pid);
        return 0;
    }
    /* iterate over the root's children */
    printk("%d [fillcolor=white, style=\"filled\", label=\"name:%s&#92;nstart time:%llu\"]", 
                root_task->pid, root_task->comm, root_task->start_time);

    
    list_for_each(list, &root_task->children){
        task = list_entry(list, struct task_struct, sibling);

        
        
        if (heir == 1){ // color heir as yellow
            printk("%d [fillcolor=yellow, style=\"filled\", label=\"name:%s&#92;nstart time:%llu\"]", 
                    task->pid, task->comm, task->start_time);

            heir = 0;
        }
        else  // color all other children as white
        {
            printk("%d [fillcolor=white, style=\"filled\", label=\"name:%s&#92;nstart time:%llu\"]", 
                task->pid, task->comm, task->start_time);

        }
        printk("%d -> %d ;", 
                root_task->pid, task->pid);
        
    }
    printk("}");
    return 0;
}

void simple_exit(void){
    printk(KERN_WARNING "Exiting kernel module\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pinar & Yigit");
module_init(simple_init);
module_exit(simple_exit);

