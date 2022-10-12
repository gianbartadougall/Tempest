/**
 * @file tempest_config.h
 * @author Gian Barta-Dougall
 * @brief This file stores the version of the project that will be compiled. Different versions
 * may change the hardware and software configurations of the project. This system thus allows
 * one project to support multiple versions of the same project at the same time
 * @version 0.1
 * @date --
 *
 * @copyright Copyright (c)
 *
 */
#ifndef VERSION_CONFIG_H
#define VERSION_CONFIG_H

/**
 * The MAJOR, MINOR, PATCH #defines represents the current version of the system in the form
 * 'version MAJOR.MINOR.PATH'.
 * MAJOR: Incremented by 1 whenever there is a hardware change that alters the configuration
 * of the GPIO pins of the microprocessor
 * MINOR: Incrememnted by 1 whenever there is a hardware change that does not alter the
 * configuration of the GPIO pins. Whilst not always the case, this generally implies an update
 * to the software i.e A model of sensor used by the system is being changed and the new model
 * sensor has the same GPIO pin configuration but requires different software to communicate
 * PATCH: Incremented by 1 when enough pure software changes (i.e general code, bug fixes, new .c/.h
 * files etc) have been made. The term 'enough' is up to the discretion of the software engineer
 *
 * The MAJOR, MINOR, PATCH numbers are used to determine what and how the hardware configurations
 * (GPIO pins, ADCs, timers etc) will be setup.
 *
 * For versions of Tempest that have different hardware configurations, change the MAJOR number
 * to select the correct hardware configuration for the given PCB to be programmed
 *
 * For versions of Tempest that have different software configurations, change the Minor and
 * PATCH numbers to select the correct software configuration for the given PCB to programmed
 *
 * The MAJOR, MINOR, PATH numbers are joined together to create a unique 16-bit version number
 * (xxxxyyyyyyzzzzzz) where x represents a 4-bit MAJOR number, y represents a 6-bit MINOR number
 * and z represents a 6-bit PATCH number.
 */
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATH  0

// Confirm the MAJOR, MINOR, PATCH numbers do not exceed their maximum values. If the MAJOR,
// MINOR or PATCH numbers exceeed their maximum values, they cannot fit into a 16-bit number
#if ((VERSION_MAJOR > 15) || (VERSION_MINOR > 127) || (VERSION_PATCH > 127))
    #error Illegal version number. MAJOR > 15 or MINOR > 127 or PATCH > 127
#endif

// Define which version of Tempest the project will compile for
#define PROJECT_VERSION ((VERSION_MAJOR << 12) | (VERSION_MINOR < 7) | (VERSION_PATCH))

/** List of all the different modules used in the project. These modules can be commented out
 *  to disable them for debugging purposes. These modules also may only be included in certain
 *  versions of the system
 */
#if (VERSION_MAJOR == 0)
    #define BUTTON_MODULE_ENABLED
    #define TASK_SCHEDULER_ENABLED
    // #define MOTOR_MODULE_ENABLED
    // #define AMBIENT_LIGHT_SENSOR_MODULE_ENABLED
    // #define LED_MODULE_ENABLED
    // #define ENCODER_MODULE_ENABLED
    // #define WINDOW_SENSOR_MODULE_ENABLED
#endif

#endif // VERSION_CONFIG_H