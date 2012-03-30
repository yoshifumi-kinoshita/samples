#include <linux/module.h>

static int __init stall_init(void)
{
  printk("stall module loading\n");
  local_irq_disable();
  while(1);
  return 0;
}
module_init(stall_init);

