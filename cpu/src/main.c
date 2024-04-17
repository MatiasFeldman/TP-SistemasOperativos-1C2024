#include <../include/cpu.h>

// inicializacion

t_log *logger_cpu;
t_config *config_cpu;
char *ip_memoria;
char *puerto_memoria;
char *puerto_dispatch;
char *puerto_interrupt;
u_int32_t conexion_memoria, conexion_kernel_dispatch, conexion_kernel_interrupt;
int socket_servidor_dispatch, socket_servidor_interrupt;
pthread_t hilo_dispatch, hilo_interrupt;

int main(void)
{
	iniciar_config();

	conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, logger_cpu);
	enviar_mensaje("", conexion_memoria, CPU, logger_cpu);

	// iniciar servidor Dispatch y Interrupt
	pthread_create(&hilo_dispatch, NULL, iniciar_servidor_dispatch, NULL);
	pthread_create(&hilo_interrupt, NULL, iniciar_servidor_interrupt, NULL);

	pthread_join(hilo_dispatch, NULL);
	pthread_join(hilo_interrupt, NULL);
}

void iniciar_config()
{
	config_cpu = config_create("./config/cpu.config");
	logger_cpu = iniciar_logger("config/cpu.log", "CPU", LOG_LEVEL_INFO);
	ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
	puerto_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
	puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
	puerto_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
}

void *iniciar_servidor_dispatch(void *arg)
{
	socket_servidor_dispatch = iniciar_servidor(logger_cpu, puerto_dispatch, "DISPATCH");
	while (1)
	{
		conexion_kernel_dispatch = esperar_cliente(socket_servidor_dispatch, logger_cpu);
		log_info(logger_cpu, "Se recibio un mensaje del modulo %s en el Dispatch", cod_op_to_string(recibir_operacion(conexion_kernel_dispatch)));
		recibir_mensaje(conexion_kernel_dispatch, logger_cpu);
	}
	return NULL;
}

void *iniciar_servidor_interrupt(void *arg)
{
	socket_servidor_interrupt = iniciar_servidor(logger_cpu, puerto_interrupt, "INTERRUPT");
	while (1)
	{
		conexion_kernel_interrupt = esperar_cliente(socket_servidor_interrupt, logger_cpu);
		log_info(logger_cpu, "Se recibio un mensaje del modulo %s en el Interrupt", cod_op_to_string(recibir_operacion(conexion_kernel_interrupt)));
		recibir_mensaje(conexion_kernel_interrupt, logger_cpu);
	}
	return NULL;
}