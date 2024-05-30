#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/temporal.h>

#define SIZE_REGISTROS 32

extern sem_t semaforo;
typedef enum
{
	SOLICITUD_INSTRUCCION,
	INSTRUCCION,
	CREAR_PROCESO,
	CPU,
	KERNEL,
	MEMORIA,
	ENTRADA_SALIDA,
	PCB,
	INTERRUPTION,
	PRUEBA,
	INTERFAZ_DIALFS,
	INTERFAZ_STDIN,
	INTERFAZ_STDOUT,
	INTERFAZ_GENERICA,
	FIN_OPERACION_IO,
	END_PROCESS,
	FIN_CLOCK,
	KILL_PROCESS,
	OPERACION_IO,
	IO_SUCCESS,
	EJECUTAR_IO,
	INTERRUPCION_CLOCK,
	CERRAR_IO,
	WAIT_SOLICITADO,
	SIGNAL_SOLICITADO


} op_code;

typedef enum
{
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS
} cod_interfaz;


typedef struct
{
	uint32_t pid;
	op_code motivo;
} t_interrupcion;

typedef struct
{
	uint32_t size;
	uint32_t offset;
	void *stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer *buffer;
} t_paquete;

typedef enum
{
	NEW,
	READY,
	EXEC,
	BLOCKED,
	TERMINATED // cambie de exit a salida porque esta tambien declarado como isntruccion
} estados;
typedef struct
{
	uint8_t AX, BX, CX, DX;
	uint32_t EAX, EBX, ECX, EDX, SI, DI, PC;
} t_registros;

typedef struct
{
	u_int32_t pid;		   //  Identificador del proceso (deberá ser un número entero, único en todo el sistema)
	u_int32_t pc;		   // Program Counter (Número de la próxima instrucción a ejecutar)
	u_int32_t quantum;	   // Unidad de tiempo utilizada por el algoritmo de planificación VRR
	t_registros registros; // Estructura que contendrá los valores de los registros de uso general de la CPU
	estados estado_actual;
} t_pcb;

typedef enum
{
	// 5 PARÁMETROS
	IO_FS_WRITE,
	IO_FS_READ,
	// 3 PARAMETROS
	IO_FS_TRUNCATE,
	IO_STDOUT_WRITE,
	IO_STDIN_READ,
	// 2 PARAMETROS
	SET,
	MOV_IN,
	MOV_OUT,
	SUM,
	SUB,
	JNZ,
	IO_GEN_SLEEP,
	IO_FS_DELETE,
	IO_FS_CREATE,
	// 1 PARAMETRO
	RESIZE,
	COPY_STRING,
	WAIT,
	SIGNAL,
	// 0 PARAMETROS
	EXIT
} t_identificador;

typedef struct
{
	t_identificador identificador;
	uint32_t param1_length;
	uint32_t param2_length;
	uint32_t param3_length;
	uint32_t param4_length;
	uint32_t param5_length;
	uint32_t cant_parametros;
	char **parametros; // Lista de parámetros

} t_instruccion;

typedef struct
{
	uint32_t pid;
	uint32_t path_length;
	char *path;
} t_solicitudCreacionProcesoEnMemoria;

typedef struct
{
	void *contenido;
	int pagina;
} t_pagina;

typedef struct
{
	t_pagina **tabla_paginas;
} t_tabla_paginas;

typedef struct
{
	uint32_t pid;
	// char* path;
	t_list *lista_instrucciones;
	t_tabla_paginas *tabla_paginas;
} t_proceso;

typedef struct
{
	uint32_t pid;
	uint32_t pc;
} t_solicitudInstruccionEnMemoria;

typedef struct
{
	uint32_t conexion;
	cod_interfaz tipo_interfaz;
} t_interfaz_en_kernel;



typedef struct
{
	t_instruccion *instruccion_io;
	uint32_t pid;
} t_instruccionEnIo;

typedef struct
{
	pthread_mutex_t mutex;
	sem_t instruccion_en_cola;
	sem_t binario_io_libre;
} t_semaforosIO;

typedef struct{
	pthread_mutex_t mutex;
	pthread_mutex_t mutex_cola_recurso;
	uint32_t instancias;
} t_recurso_en_kernel;

typedef struct{
	char* nombre_recurso;
	uint32_t instancias_asignadas;
} t_recurso_asignado_a_proceso;

void terminar_programa(int conexion, t_log *logger, t_config *config);

char *cod_op_to_string(op_code codigo_operacion);
t_registros inicializar_registros();
void set_registro(t_registros *registros, char *registro, u_int32_t valor);
u_int8_t get_registro_int8(t_registros *registros, char *registro);
u_int32_t get_registro_int32(t_registros *registros, char *registro);
void sum_registro(t_registros *registros, char *registroOrigen, char *registroDestino);
void sub_registro(t_registros *registros, char *registroOrigen, char *registroDestino);
void JNZ_registro(t_registros *registros, char *registro, u_int32_t valor);
void imprimir_registros_por_pantalla(t_registros registros);
t_pcb *crear_pcb(u_int32_t pid, u_int32_t quantum, estados estado);
t_pcb *recibir_pcb(int socket);
void destruir_pcb(t_pcb *pcb);
void enviar_pcb(t_pcb *pcb, int socket);
t_log *iniciar_logger(char *path, char *nombre, t_log_level nivel);
int crear_conexion(char *ip, char *puerto, t_log *logger);
void liberar_conexion(int socket_cliente);
int iniciar_servidor(t_log *logger, char *puerto, char *nombre);
int esperar_cliente(int socket_servidor, t_log *logger);
void *serializar_paquete(t_paquete *paquete, int bytes);
t_paquete *crear_paquete(op_code codigo_operacion);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
char *recibir_mensaje_guardar_variable(int socket_cliente);

void enviar_mensaje(char *mensaje, int socket_cliente, op_code codigo_operaciom, t_log *logger);
void enviar_codigo_operacion(op_code code, int socket);
void eliminar_paquete(t_paquete *paquete);
op_code recibir_operacion(int socket_cliente);
void *recibir_buffer(int *size, int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log *logger);
t_paquete *recibir_paquete(int socket_cliente);
void buffer_read(t_buffer *buffer, void *data, uint32_t size);
void buffer_add(t_buffer *buffer, void *data, uint32_t size);
t_buffer *crear_buffer();
void destruir_buffer(t_buffer *buffer);
t_buffer *serializar_instruccion(t_instruccion *instruccion);
t_instruccion *instruccion_deserializar(t_buffer *buffer, u_int32_t offset);
t_buffer *serializar_lista_instrucciones(t_list *lista_instrucciones);
t_list *deserializar_lista_instrucciones(t_buffer *buffer, u_int32_t offset);
void imprimir_instruccion(t_instruccion instruccion);
void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion);
void destruir_instruccion(t_instruccion *instruccion);

t_instruccion *crear_instruccion(t_identificador identificador, t_list *parametros);
t_buffer *serializar_solicitud_crear_proceso(t_solicitudCreacionProcesoEnMemoria *solicitud);
t_solicitudCreacionProcesoEnMemoria *deserializar_solicitud_crear_proceso(t_buffer *buffer);
void imprimir_proceso(t_proceso *proceso);
t_list *parsear_instrucciones(FILE *archivo_instrucciones);
t_identificador string_to_identificador(char *string);
void imprimir_lista_de_procesos(t_list *lista_procesos);
t_interrupcion *deserializar_interrupcion(t_buffer *buffer);
t_buffer *serializar_interrupcion(t_interrupcion *interrupcion);
void enviar_motivo_desalojo(op_code motivo, uint32_t socket);
op_code recibir_motivo_desalojo(uint32_t socket_cliente);
void enviar_interrupcion(u_int32_t pid,op_code interrupcion_code,u_int32_t socket);
cod_interfaz cod_op_to_tipo_interfaz(op_code cod_op);
t_instruccionEnIo *deserializar_instruccion_en_io(t_buffer *buffer);
t_buffer *serializar_instruccion_en_io(t_instruccionEnIo *instruccion);
op_code tipo_interfaz_to_cod_op(cod_interfaz tipo);
#endif /* UTILS_H_ */