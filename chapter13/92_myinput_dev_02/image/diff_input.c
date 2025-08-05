linux_sdk/kernel$ git diff drivers/input/input.c
diff --git a/drivers/input/input.c b/drivers/input/input.c
index f96c40aed..b12e21bc1 100755
--- a/drivers/input/input.c
+++ b/drivers/input/input.c
@@ -984,8 +984,10 @@ bool input_match_device_id(const struct input_dev *dev,
		            !bitmap_subset(id->ffbit, dev->ffbit, FF_MAX) ||
			                !bitmap_subset(id->swbit, dev->swbit, SW_MAX) ||
					            !bitmap_subset(id->propbit, dev->propbit, INPUT_PROP_MAX)) {
	+               printk("input dev is error %s\n",dev->name);
	                return false;
			        }
+       printk("input dev is ok %s\n",dev->name);
 
        return true;
	 }
@@ -995,6 +997,7 @@ static const struct input_device_id *input_match_device(struct input_handler *ha
		                                                        struct input_dev *dev)
 {
	         const struct input_device_id *id;
		 +       printk("handler name is %s\n",handler->name);
		  
		         for (id = handler->id_table; id->flags || id->driver_info; id++) {
				                 if (input_match_device_id(dev, id) &&
