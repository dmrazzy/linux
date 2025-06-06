// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
//
// Copyright 2019 NXP
//
// Author: Daniel Baluta <daniel.baluta@nxp.com>
//

#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pm_runtime.h>
#include <sound/sof.h>

#include "sof-of-dev.h"
#include "ops.h"

static char *fw_path;
module_param(fw_path, charp, 0444);
MODULE_PARM_DESC(fw_path, "deprecated - moved to snd-sof module.");

static char *fw_filename;
module_param(fw_filename, charp, 0444);
MODULE_PARM_DESC(fw_filename, "deprecated - moved to snd-sof module.");

static char *tplg_path;
module_param(tplg_path, charp, 0444);
MODULE_PARM_DESC(tplg_path, "deprecated - moved to snd-sof module.");

static char *tplg_filename;
module_param(tplg_filename, charp, 0444);
MODULE_PARM_DESC(tplg_filename, "deprecated - moved to snd-sof module.");

EXPORT_DEV_PM_OPS(sof_of_pm) = {
	.prepare = snd_sof_prepare,
	.complete = snd_sof_complete,
	SYSTEM_SLEEP_PM_OPS(snd_sof_suspend, snd_sof_resume)
	RUNTIME_PM_OPS(snd_sof_runtime_suspend, snd_sof_runtime_resume, NULL)
};

static void sof_of_probe_complete(struct device *dev)
{
	/* allow runtime_pm */
	pm_runtime_set_autosuspend_delay(dev, SND_SOF_SUSPEND_DELAY_MS);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_mark_last_busy(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
}

int sof_of_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct sof_dev_desc *desc;
	struct snd_sof_pdata *sof_pdata;

	dev_info(&pdev->dev, "DT DSP detected");

	sof_pdata = devm_kzalloc(dev, sizeof(*sof_pdata), GFP_KERNEL);
	if (!sof_pdata)
		return -ENOMEM;

	desc = device_get_match_data(dev);
	if (!desc)
		return -ENODEV;

	if (!desc->ops) {
		dev_err(dev, "error: no matching DT descriptor ops\n");
		return -ENODEV;
	}

	sof_pdata->desc = desc;
	sof_pdata->dev = &pdev->dev;

	sof_pdata->ipc_file_profile_base.ipc_type = desc->ipc_default;
	sof_pdata->ipc_file_profile_base.fw_path = fw_path;
	sof_pdata->ipc_file_profile_base.tplg_path = tplg_path;
	sof_pdata->ipc_file_profile_base.fw_name = fw_filename;
	sof_pdata->ipc_file_profile_base.tplg_name = tplg_filename;

	/* set callback to be called on successful device probe to enable runtime_pm */
	sof_pdata->sof_probe_complete = sof_of_probe_complete;

	/* call sof helper for DSP hardware probe */
	return snd_sof_device_probe(dev, sof_pdata);
}
EXPORT_SYMBOL(sof_of_probe);

void sof_of_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);

	/* call sof helper for DSP hardware remove */
	snd_sof_device_remove(&pdev->dev);
}
EXPORT_SYMBOL(sof_of_remove);

void sof_of_shutdown(struct platform_device *pdev)
{
	snd_sof_device_shutdown(&pdev->dev);
}
EXPORT_SYMBOL(sof_of_shutdown);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("SOF support for OF/DT platforms");
