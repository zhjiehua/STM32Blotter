
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#define backflow_TASK_PRIORITY          ( tskIDLE_PRIORITY + 1 )
#define calibration_TASK_PRIORITY       ( tskIDLE_PRIORITY + 1 )
#define purge_TASK_PRIORITY             ( tskIDLE_PRIORITY + 1 )
#define project_TASK_PRIORITY           ( tskIDLE_PRIORITY + 1 )
#define home_TASK_PRIORITY              ( tskIDLE_PRIORITY + 2 )
#define ui_TASK_PRIORITY                ( tskIDLE_PRIORITY + 1 )

#define backflow_TASK_STACKDEP          ( 100 )
#define calibration_TASK_STACKDEP       ( 100 )
#define purge_TASK_STACKDEP             ( 100 )
#define project_TASK_STACKDEP           ( 100 )
#define home_TASK_STACKDEP              ( 100 )
#define ui_TASK_STACKDEP                ( 100 )

void CalibrationTask(void);
void PurgeTask(void);
void ProjectTask(void);
void HomeTask(void);
void UITask(void);

	


