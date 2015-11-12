/*
 * This is a virtual media controller device driver for testing applications
 * and userspace libraries of media devices.
 *
 * Copyright (c) 2015.
 * Navya Sri Nizamakri, <navysri.tech@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version
 */
#include <linux/init.h>
#include <linux/media.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <media/media-device.h>
#include <media/media-devnode.h>
#include <media/media-entity.h>

#define VIMC_NAME  "vimc"

struct driver_data {
       struct media_device mdev;
       struct media_entity entity[4];
       struct media_pad pads[6];
};

static void mdev_dev_release(struct device *dev)
{
}

static int med_remove(struct platform_device *pdev)
{
       unsigned int i;
       struct driver_data *drvdata = platform_get_drvdata(pdev);

       media_device_unregister(&drvdata->mdev);
       for (i = 0; i < ARRAY_SIZE(drvdata->entity); i++)
               media_entity_cleanup(&drvdata->entity[i]);

       return 0;
}

static int med_probe(struct platform_device *pdev)
{
       unsigned int i;
       unsigned int num_pads;
       int ret;
       struct driver_data *drvdata;
       struct media_pad *padp;

       drvdata = devm_kzalloc(&pdev->dev, sizeof(*drvdata), GFP_KERNEL);
       if (!drvdata)
               return -ENOMEM;

       drvdata->mdev.dev = &pdev->dev;
       drvdata->mdev.model[0] = 'v';

       ret = media_device_register(&drvdata->mdev);
       if (ret < 0) {
               dev_err(&pdev->dev, "media device registration failed (%d)\n",
                       ret);
               return ret;
       }

       drvdata->pads[0].flags = MEDIA_PAD_FL_SOURCE;
       drvdata->pads[1].flags = MEDIA_PAD_FL_SOURCE;
       drvdata->pads[2].flags = MEDIA_PAD_FL_SINK;
       drvdata->pads[3].flags = MEDIA_PAD_FL_SOURCE;
       drvdata->pads[4].flags = MEDIA_PAD_FL_SINK;
       drvdata->pads[5].flags = MEDIA_PAD_FL_SINK;
       padp = &drvdata->pads[0];

       for (i = 0; i < ARRAY_SIZE(drvdata->entity); i++) {
               if (i == 0 || i == 3)
                       num_pads = 1;
               else
                       num_pads = 2;

               media_entity_init(&drvdata->entity[i], num_pads, padp, 0);
               padp += num_pads;
               ret = media_device_register_entity(&drvdata->mdev,
                                                  &drvdata->entity[i]);
               if (ret < 0)
                       goto error;
       }

       media_entity_create_link(&drvdata->entity[0], 0,
                                &drvdata->entity[1], 1, 0);
       media_entity_create_link(&drvdata->entity[1], 0,
                                &drvdata->entity[2], 1, 0);
       media_entity_create_link(&drvdata->entity[2], 0,
                                &drvdata->entity[3], 0, 0);
       platform_set_drvdata(pdev, drvdata);

       return 0;

error:
       for (i = 0; i < ARRAY_SIZE(drvdata->entity); i++)
               media_entity_cleanup(&drvdata->entity[i]);

       return ret;
}

static struct platform_device med_pdev = {
       .name           = VIMC_NAME,
       .dev.release    = mdev_dev_release,
};

static struct platform_driver med_pdrv = {
       .probe          = med_probe,
       .remove         = med_remove,
       .driver         = {
               .name   = VIMC_NAME,
       },
};

static int __init vimc_init(void)
{
       int ret;

       platform_device_register(&med_pdev);
       ret = platform_driver_register(&med_pdrv);
       if (ret)
               platform_device_unregister(&med_pdev);

       return 0;
}

static void __exit vimc_exit(void)
{
       platform_driver_unregister(&med_pdrv);
       platform_device_unregister(&med_pdev);
}

module_init(vimc_init);
module_exit(vimc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Navya Sri Nizamakri, <navyasri.tech@gmail.com>");
MODULE_DESCRIPTION("This is a virtual media controller device driver");
