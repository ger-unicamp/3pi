/* Algoritimo para resolução de labirinto usando Aprendizado de Máquina 
* Proposto por Claude Shannon em seu exerimento do Theseus 
* Simularemos com o 3pi como sendo Theseus 
* O Robô percorrerá o labirinto salvando seus erros e acertos, ao final, quando colocado novamente ele já saberá o menor cmainho para saida 
* Autor: Leonardo Alves de Melo */

/* Biblioteca padrao do 3pi */
#include <pololu/3pi.h>

/* Outras libs */
#include <avr/pgmspace.h>
#include "bargraph.h"
#include "maze-solve.h"

/* Sera printado na tela LCD ao iniciar o programa*/
const char welcome_line1[] PROGMEM = " GER";
const char welcome_line2[] PROGMEM = "Shannon";
const char demo_name_line1[] PROGMEM = "Maze";
const char demo_name_line2[] PROGMEM = "solver";

/* Musicas do robo */
const char welcome[] PROGMEM = ">g32>>c32";
const char go[] PROGMEM = "L16 cdegreg4";


/* Definimos a direcao do robo */
#define NORTE 0
#define LESTE 1
#define SUL 2
#define OESTE 3

#define HORARIO 0
#define ANTI_HORARIO 1

/* Numero de inicializacao do vetor de erros de uma posicao do labirinto e da direcao certa */
#define INCERTO 5

#define N_ORIENTACOES 4

/* Posicao inicial do mapa do robo */
#define X_ROBO 0
#define Y_ROBO 0

/* Posicao da saida */
#define X_SAIDA 10
#define Y_SAIDA 10

#define ORIENTACAO_INICIAL NORTE

/* Neste struct está o percurso certo para sair do labirinto e os lados errados */
typedef struct Percurso {
	int direcao_certa;
	int teste_anterior;
	bool erro[N_ORIENTACOES];
	struct Percurso *prox;
	struct Percurso *anterior;
} Percurso;

/* Vetor 2D que guarda a posicao do robo no mapa e/ou a posicao da saida */
typedef struct Mapa {
	int x;
	int y;
} Mapa;

/* Posicao inicial do robo */
Percurso posicao_inicial;

Percurso *posicao_atual;

/* Local onde o robo esta no mapa */
Mapa local_robo;

/* Local onde está a saida */
Mapa local_saida;

/* Variavel que guarda a orientacao do 3pi */
int orientacao = ORIENTACAO_INICIAL;

/* Inicializa o robo, mostra uma mensagem, calibra os sensores e toca uma musica */
/* Este codigo eh do 3pi */
void inicializa() {

	unsigned int counter; // used as a simple timer
	unsigned int sensors[5]; // an array to hold sensor values

	// This must be called at the beginning of 3pi code, to set up the
	// sensors.  We use a value of 2000 for the timeout, which
	// corresponds to 2000*0.4 us = 0.8 ms on our 20 MHz processor.
	pololu_3pi_init(2000);
	load_custom_characters(); // load the custom characters
	
	// Play welcome music and display a message
	print_from_program_space(welcome_line1);
	lcd_goto_xy(0,1);
	print_from_program_space(welcome_line2);
	play_from_program_space(welcome);
	delay_ms(1000);

	clear();
	print_from_program_space(demo_name_line1);
	lcd_goto_xy(0,1);
	print_from_program_space(demo_name_line2);
	delay_ms(1000);

	// Display battery voltage and wait for button press
	while(!button_is_pressed(BUTTON_B))
	{
		int bat = read_battery_millivolts();

		clear();
		print_long(bat);
		print("mV");
		lcd_goto_xy(0,1);
		print("Press B");

		delay_ms(100);
	}

	// Always wait for the button to be released so that 3pi doesn't
	// start moving until your hand is away from it.
	wait_for_button_release(BUTTON_B);
	delay_ms(1000);

	// Auto-calibration: turn right and left while calibrating the
	// sensors.
	for(counter=0;counter<80;counter++)
	{
		if(counter < 20 || counter >= 60)
			set_motors(40,-40);
		else
			set_motors(-40,40);

		// This function records a set of sensor readings and keeps
		// track of the minimum and maximum values encountered.  The
		// IR_EMITTERS_ON argument means that the IR LEDs will be
		// turned on during the reading, which is usually what you
		// want.
		calibrate_line_sensors(IR_EMITTERS_ON);

		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay_ms(20);
	}
	set_motors(0,0);

	// Display calibrated values as a bar graph.
	while(!button_is_pressed(BUTTON_B))
	{
		// Read the sensor values and get the position measurement.
		unsigned int position = read_line(sensors,IR_EMITTERS_ON);

		// Display the position measurement, which will go from 0
		// (when the leftmost sensor is over the line) to 4000 (when
		// the rightmost sensor is over the line) on the 3pi, along
		// with a bar graph of the sensor readings.  This allows you
		// to make sure the robot is ready to go.
		clear();
		print_long(position);
		lcd_goto_xy(0,1);
		display_readings(sensors);

		delay_ms(100);
	}
	wait_for_button_release(BUTTON_B);

	clear();

	print("GER!");		

	// Play music and wait for it to finish before we start driving.
	play_from_program_space(go);
	while(is_playing());

}

/* Inicializa a posicao criada */
void inicializa_posicao(Percurso *posicao, Percurso *anterior) {

	int i;

	posicao->direcao_certa = INCERTO;

	for(i = 0; i < N_ORIENTACOES; i++) {
		posicao->erro[i] = false;
	}

	posicao->prox = NULL;

	posicao->anterior = anterior;

}

/* Configuracoes inicial do local do robo e da saida */
void inicializa_mapa() {

	local_robo.x = X_ROBO;
	local_robo.y = Y_ROBO;

	local_saida.x = X_SAIDA;
	local_saida.y = Y_SAIDA;
}

/* Gira 90 graus para um snetido */
void girar(int sentido) {

	if(sentido == HORARIO) {

	}
	else {

	}
}

void andar_frente() {

}

void andar_tras() {

}

/* Troca a orientacao para uma nova, girando o robo */
void troca_orientacao(int nova_orientacao) {

	if(orientacao == NORTE) {

		if(nova_orientacao == LESTE) {
			gira(HORARIO);
		}
		else if(nova_orientacao == SUL) {
			gira(HORARIO);
			gira(HORARIO);
		}
		else if(nova_orientacao == OESTE) {
			gira(ANTI_HORARIO);
		}
	}
	else if(orientacao == SUL) {

		if(nova_orientacao == LESTE) {
			gira(ANTI_HORARIO);
		}
		else if(nova_orientacao == NORTE) {
			gira(HORARIO);
			gira(HORARIO);
		}
		else if(nova_orientacao == OESTE) {
			gira(HORARIO);
		}
	} 
	else if(orientacao == LESTE) {

		if(nova_orientacao == NORTE) {
			gira(ANTI_HORARIO);
		}
		else if(nova_orientacao == SUL) {
			gira(HORARIO);
		}
		else if(nova_orientacao == OESTE) {
			gira(HORARIO);
			gira(HORARIO);
		}
	}
	else if(orientacao == OESTE) {

		if(nova_orientacao == LESTE) {
			gira(HORARIO);
			gira(HORARIO);
		}
		else if(nova_orientacao == SUL) {
			gira(ANTI_HORARIO);
		}
		else if(nova_orientacao == NORTE) {
			gira(HORARIO);
		}
	}

}

/* Retorna true se nao ha parede na frente dele e fica na nova posicao, e false se ha */
bool testa_frente() {

}

/* Cria uma nova posicao a partir de onde esta */
void cria_nova_posicao(int incremento_x, int incremento_y, int orientacao) {

	local_robo.x += incremento_x;
	local_robo.y += incremento_y;

	posicao_atual->prox = (Percurso *)malloc(sizeof(Percurso));	

	inicializa_posicao(posicao_atual->prox, posicao_atual, orientacao);

	posicao_atual = posicao_atual->prox;

	posicao_atual->teste_anterior = orientacao;
}

void volta_posicao() {

	if(posicao_atual->teste_anterior == NORTE) {
		troca_orientacao(SUL);
		testa_frente();
		local_robo.y += -1;
	}
	else if(posicao_atual->teste_anterior == SUL) {
		troca_orientacao(NORTE);
		testa_frente();
		local_robo.y += 1;
	}
	else if(posicao_atual->teste_anterior == LESTE) {
		troca_orientacao(OESTE);
		testa_frente();
		local_robo.x += -1;
	}
	else if(posicao_atual->teste_anterior == OESTE) {
		troca_orientacao(LESTE);
		testa_frente();
		local_robo.x += 1;
	}

	posicao_atual = posicao_atual->anterior;

	if(posicao_atual != NULL) {		
		free(posicao_atual->prox);
	}
	prosicao_atual->prox = NULL
}

/* Tenta achar a alguma saida para aquela posicao */
void testa_saida() {

	/* Primeiro testa o sentido Norte */
	if(posicao_atual->erro[NORTE] == false) {
		troca_orientacao(NORTE);

		/* Checa se nao ha parede a frente */
		if(testa_frente()) {
			cria_nova_posicao(0, 1, NORTE);
		}
		/* Se tiver, declara que nao pode ir para aquela posicao */
		else {
			posicao_atual->erro[NORTE] = true;
		}
	}
	else if(posicao_atual->erro[LESTE] == false) {
		troca_orientacao(LESTE)

		/* Checa se nao ha parede a frente */
		if(testa_frente()) {
			cria_nova_posicao(1, 0, LESTE);
		}
		/* Se tiver, declara que nao pode ir para aquela posicao */
		else {
			posicao_atual->erro[LESTE] = true;
		}
	}
	else if(posicao_atual->erro[SUL] == false) {
		troca_orientacao(SUL)

		/* Checa se nao ha parede a frente */
		if(testa_frente()) {
			cria_nova_posicao(0, -1, SUL);
		}
		/* Se tiver, declara que nao pode ir para aquela posicao */
		else {
			posicao_atual->erro[SUL] = true;
		}
	}
	else if(posicao_atual->erro[OESTE] == false) {
		troca_orientacao(OESTE)

		/* Checa se nao ha parede a frente */
		if(testa_frente()) {
			cria_nova_posicao(-1, 0, OESTE);
		}
		/* Se tiver, declara que nao pode ir para aquela posicao */
		else {
			posicao_atual->erro[OESTE] = true;
		}
	}
	/* Se nenhum dos locais estiver certo, entao volta uma posicao */
	else {
		volta_posicao();
	}
}

int main() {


	/* Inicializa o 3pi */
	inicializa();

	/* Inicializa a posicao inicial */
	inicializa_posicao(&posicao_inicial, NULL);
	posicao_atual = &posicao_inicial;

	inicializa_mapa();

	/* Começa o algoritmo */
	testa_saida();

	return 0;
}

