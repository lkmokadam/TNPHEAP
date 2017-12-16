//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng
//
//   Description:
//     Skeleton of NPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "tnpheap_ioctl.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/time.h>

struct miscdevice tnpheap_dev;

static DEFINE_MUTEX(commit_lock);


struct tnp_node{
    long unsigned int offset; 
    unsigned int version; 
    struct list_head list;
};

struct list_head tnp_addr_list ;
LIST_HEAD(tnp_addr_list);

struct tnp_node *tmp; 
struct list_head *pos; 


__u64 tnpheap_get_version(struct tnpheap_cmd __user *user_cmd)
{
    
    struct tnpheap_cmd cmd;
    struct tnp_node *tmp;
    struct list_head *pos;
    
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return (__u64)-1 ;
    } 
    mutex_lock(&commit_lock);
    cmd.version = 0;
    list_for_each(pos, &tnp_addr_list)
    {
        // getting the node from the list 
        tmp = list_entry(pos, struct tnp_node, list);
        // checking for object id
        if (tmp->offset == cmd.offset)
        {
            // if found the objectid, return version
            cmd.version = tmp->version;
            break;
        }
    }
    mutex_unlock(&commit_lock);
    return cmd.version;
}

__u64 tnpheap_start_tx(struct tnpheap_cmd __user *user_cmd)
{
    struct tnpheap_cmd cmd; 
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return (__u64)-1 ;
    } 
    return (__u64)0;

}

__u64 tnpheap_commit(struct tnpheap_cmd __user *user_cmd)
{
    
     __u64 ret=0;
    
    struct tnpheap_cmd cmd;
    struct tnp_node *tmp;
    struct list_head *pos;  
    
   
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return (__u64)-1 ;
    }
    
    mutex_lock(&commit_lock);
    list_for_each(pos, &tnp_addr_list)
    {
        // getting the node from the list 
        tmp = list_entry(pos, struct tnp_node, list);
        // checking for object id
        if (tmp->offset == cmd.offset)
        {
            // if found the objectid, return , else -1
            if(tmp->version == cmd.version ){
                 ret= 0;
                 mutex_unlock(&commit_lock);
                 return ret;
            }
            else{
                ret= -1;
                mutex_unlock(&commit_lock);
                return ret;
            }
                
        }
    }
    
    tmp = (struct tnp_node *)kmalloc(sizeof(struct tnp_node),GFP_KERNEL);
    tmp->offset = cmd.offset;
    tmp->version=0;
    list_add(&(tmp->list), &(tnp_addr_list));
    mutex_unlock(&commit_lock);
    ret= 0;
    return ret;

}



long tnpheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case TNPHEAP_IOCTL_START_TX:
        return tnpheap_start_tx((void __user *) arg);
    case TNPHEAP_IOCTL_GET_VERSION:
        return tnpheap_get_version((void __user *) arg);
    case TNPHEAP_IOCTL_COMMIT:
        return tnpheap_commit((void __user *) arg);
    default:
        return -ENOTTY;
   }
}

static const struct file_operations tnpheap_fops = {
    .owner                = THIS_MODULE,
    .unlocked_ioctl       = tnpheap_ioctl,
};

struct miscdevice tnpheap_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "tnpheap",
    .fops = &tnpheap_fops,
};

static int __init tnpheap_module_init(void)
{
    int ret = 0;
    if ((ret = misc_register(&tnpheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else
        printk(KERN_ERR "\"npheap\" misc device installed\n");
    return 1;
}

static void __exit tnpheap_module_exit(void)
{
    misc_deregister(&tnpheap_dev);
    return;
}

MODULE_AUTHOR("Hung-Wei Tseng <htseng3@ncsu.edu>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(tnpheap_module_init);
module_exit(tnpheap_module_exit);
