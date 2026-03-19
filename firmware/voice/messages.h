/**
 * @file messages.h
 * @brief Definición de IDs de mensajes de voz
 * 
 * Este archivo contiene los identificadores de todos los mensajes
 * de voz predefinidos utilizados por el sistema.
 * 
 * @version 1.0
 * @date Marzo 2026
 * @author Enrique Aguayo H.
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * MENSAJES GENERALES
 * ================================================================== */

#define MSG_STARTUP                 0   // Sistema iniciado
#define MSG_SHUTDOWN                 1   // Sistema apagándose
#define MSG_WELCOME                   2   // Bienvenida
#define MSG_GOODBYE                   3   // Despedida
#define MSG_READY                      4   // Listo para ayudar
#define MSG_PROCESSING                5   // Procesando...

/* ==================================================================
 * MENSAJES DE BATERÍA
 * ================================================================== */

#define MSG_BATTERY_LOW              10  // Batería baja
#define MSG_BATTERY_CRITICAL          11  // Batería crítica
#define MSG_BATTERY_CHARGING          12  // Cargando
#define MSG_BATTERY_FULL              13  // Carga completa
#define MSG_BATTERY_SHUTDOWN          14  // Apagando por batería

/* ==================================================================
 * MENSAJES DE CAÍDAS
 * ================================================================== */

#define MSG_FALL_DETECTED             20  // Caída detectada
#define MSG_FALL_ASK_STATUS           21  // ¿Estás bien?
#define MSG_FALL_CONFIRMED            22  // Caída confirmada, avisando
#define MSG_FALL_NO_RESPONSE          23  // No responde, avisando a emergencias
#define MSG_FALL_CAREGIVER_ALERT      24  // Avisando a cuidador

/* ==================================================================
 * MENSAJES DE GLUCOSA
 * ================================================================== */

#define MSG_GLUCOSE_READING           30  // Leyendo glucosa
#define MSG_GLUCOSE_NORMAL             31  // Glucosa normal
#define MSG_GLUCOSE_LOW                 32  // Glucosa baja
#define MSG_GLUCOSE_CRITICAL           33  // Glucosa crítica
#define MSG_GLUCOSE_HIGH                34  // Glucosa alta
#define MSG_GLUCOSE_TREND               35  // Tendencia de glucosa
#define MSG_GLUCOSE_REMINDER            36  // Recordatorio de medir

/* ==================================================================
 * MENSAJES DE RECETAS
 * ================================================================== */

#define MSG_RECIPE_SEARCH              40  // Buscando receta
#define MSG_RECIPE_FOUND                41  // Receta encontrada
#define MSG_RECIPE_INGREDIENTS          42  // Lista de ingredientes
#define MSG_RECIPE_STEPS                43  // Pasos de la receta
#define MSG_RECIPE_MISSING               44  // Faltan ingredientes
#define MSG_RECIPE_NOT_FOUND             45  // Receta no encontrada

/* ==================================================================
 * MENSAJES DE FAMILIA
 * ================================================================== */

#define MSG_FAMILY_GREETING             50  // Saludo familiar
#define MSG_FAMILY_VOICE                 51  // Mensaje de familiar
#define MSG_FAMILY_RECOGNITION           52  // Reconociendo familiar
#define MSG_FAMILY_NOT_RECOGNIZED        53  // Familiar no reconocido
#define MSG_FAMILY_VISIT                  54  // Aviso de visita

/* ==================================================================
 * MENSAJES DE ORIENTACIÓN
 * ================================================================== */

#define MSG_ORIENTATION_HOME             60  // Estás en casa
#define MSG_ORIENTATION_DAY               61  // Es de día
#define MSG_ORIENTATION_NIGHT             62  // Es de noche
#define MSG_ORIENTATION_DATE              63  // Fecha actual
#define MSG_ORIENTATION_TIME              64  // Hora actual
#define MSG_ORIENTATION_PLACE             65  // Lugar actual

/* ==================================================================
 * MENSAJES DE RECORDATORIOS
 * ================================================================== */

#define MSG_REMINDER_MEDICINE             70  // Recordatorio de medicación
#define MSG_REMINDER_WATER                 71  // Recordatorio de beber agua
#define MSG_REMINDER_MEAL                   72  // Recordatorio de comida
#define MSG_REMINDER_APPOINTMENT           73  // Recordatorio de cita
#define MSG_REMINDER_CALL                    74  // Recordatorio de llamada

/* ==================================================================
 * MENSAJES DE AYUDA
 * ================================================================== */

#define MSG_HELP_GENERAL                   80  // Ayuda general
#define MSG_HELP_MEDICINE                   81  // Ayuda con medicación
#define MSG_HELP_RECIPE                      82  // Ayuda con recetas
#define MSG_HELP_FAMILY                       83  // Ayuda con familiares
#define MSG_HELP_DEVICE                        84  // Ayuda con el dispositivo

/* ==================================================================
 * MENSAJES DE ERROR
 * ================================================================== */

#define MSG_ERROR_GENERAL                   90  // Error general
#define MSG_ERROR_NO_COMMAND                 91  // Comando no reconocido
#define MSG_ERROR_NO_MESSAGE                  92  // Mensaje no encontrado
#define MSG_ERROR_NO_SD                        93  // Tarjeta SD no encontrada
#define MSG_ERROR_NO_SENSOR                   94  // Sensor no disponible
#define MSG_ERROR_COMMUNICATION               95  // Error de comunicación

/* ==================================================================
 * MENSAJES DE CONFIRMACIÓN
 * ================================================================== */

#define MSG_CONFIRM_YES                      100  // Sí / Confirmado
#define MSG_CONFIRM_NO                        101  // No / Cancelado
#define MSG_CONFIRM_OK                         102  // OK
#define MSG_CONFIRM_DONE                       103  // Listo / Completado

/* ==================================================================
 * MACROS DE UTILIDAD
 * ================================================================== */

/**
 * @brief Verifica si un ID es válido
 */
#define MSG_IS_VALID(id) ((id) >= 0 && (id) <= 255)

/**
 * @brief Obtiene la categoría de un mensaje
 */
#define MSG_CATEGORY(id) ((id) / 10)

// Categorías
#define MSG_CAT_GENERAL     0
#define MSG_CAT_BATTERY     1
#define MSG_CAT_FALL        2
#define MSG_CAT_GLUCOSE     3
#define MSG_CAT_RECIPE      4
#define MSG_CAT_FAMILY      5
#define MSG_CAT_ORIENTATION 6
#define MSG_CAT_REMINDER    7
#define MSG_CAT_HELP        8
#define MSG_CAT_ERROR       9
#define MSG_CAT_CONFIRM    10

#ifdef __cplusplus
}
#endif

#endif /* MESSAGES_H */
