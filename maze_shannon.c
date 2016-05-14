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
#include "follow-segment.h"

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

#define ORIENTACAO_INICIAL NORTE

/* Definicoes sobre o tamanho do mapa */
#define LARGURA 10
#define ALTURA 10
#define TAM_MAPA LARGURA * ALTURA

/* guardamos o caminho em um vetor de caracter */
char path[TAM_MAPA] = "";
/* Cada local descoberto aumentamos o tamanho do caminho */
unsigned char path_length = 0;

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

/* Funcao que testa um caminho */
char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right)
{
	// Make a decision about how to turn.  The following code
	// implements a left-hand-on-the-wall strategy, where we always
	// turn as far to the left as possible.
	if(found_left)
		return 'L';
	else if(found_straight)
		return 'S';
	else if(found_right)
		return 'R';
	else
		return 'B';
}

/* Funcao que simplifica o caminho */
void simplify_path()
{
	// only simplify the path if the second-to-last turn was a 'B'
	if(path_length < 3 || path[path_length-2] != 'B')
		return;

	int total_angle = 0;
	int i;
	for(i=1;i<=3;i++)
	{
		switch(path[path_length-i])
		{
		case 'R':
			total_angle += 90;
			break;
		case 'L':
			total_angle += 270;
			break;
		case 'B':
			total_angle += 180;
			break;
		}
	}

	// Get the angle as a number between 0 and 360 degrees.
	total_angle = total_angle % 360;

	// Replace all of those turns with a single one.
	switch(total_angle)
	{
	case 0:
		path[path_length - 3] = 'S';
		break;
	case 90:
		path[path_length - 3] = 'R';
		break;
	case 180:
		path[path_length - 3] = 'B';
		break;
	case 270:
		path[path_length - 3] = 'L';
		break;
	}

	// The path is now two steps shorter.
	path_length -= 2;
}


/* Troca a orientacao para uma nova */
void troca_orientacao(char nova_orientacao) {

	if(orientacao == NORTE) {

		if(nova_orientacao == 'R') {
			orientacao = LESTE;
		}
		else if(nova_orientacao == 'L') {
			orientacao = OESTE;
		}
	}
	else if(orientacao == SUL) {

		if(nova_orientacao == 'R') {
			orientacao = OESTE;
		}
		else if(nova_orientacao == 'L') {
			orientacao = LESTE;
		}
	} 
	else if(orientacao == LESTE) {

		if(nova_orientacao == 'R') {
			orientacao = SUL;
		}
		else if(nova_orientacao == 'L') {
			orientacao = NORTE;
		}
	}
	else if(orientacao == OESTE) {

		if(nova_orientacao == 'R') {
			orientacao = NORTE;
		}
		else if(nova_orientacao == 'L') {
			orientacao = SUL;
		}
	}

}



/* Tenta achar a alguma saida para aquela posicao */
void resolve_e_aprende() {

	// Loop until we have solved the maze.
	while(1)
	{
		// FIRST MAIN LOOP BODY  
		follow_segment();

		// Drive straight a bit.  This helps us in case we entered the
		// intersection at an angle.
		// Note that we are slowing down - this prevents the robot
		// from tipping forward too much.
		set_motors(50,50);
		delay_ms(50);

		// These variables record whether the robot has seen a line to the
		// left, straight ahead, and right, whil examining the current
		// intersection.
		unsigned char found_left=0;
		unsigned char found_straight=0;
		unsigned char found_right=0;

		// Now read the sensors and check the intersection type.
		unsigned int sensors[5];
		read_line(sensors,IR_EMITTERS_ON);

		// Check for left and right exits.
		if(sensors[0] > 100)
			found_left = 1;
		if(sensors[4] > 100)
			found_right = 1;

		// Drive straight a bit more - this is enough to line up our
		// wheels with the intersection.
		set_motors(40,40);
		delay_ms(200);

		// Check for a straight exit.
		read_line(sensors,IR_EMITTERS_ON);
		if(sensors[1] > 200 || sensors[2] > 200 || sensors[3] > 200)
			found_straight = 1;

		// Check for the ending spot.
		// If all three middle sensors are on dark black, we have
		// solved the maze.
		if(sensors[1] > 600 && sensors[2] > 600 && sensors[3] > 600) {
			/* Tocar buzzer aqui */
			break;
		}

		// Intersection identification is complete.
		// If the maze has been solved, we can follow the existing
		// path.  Otherwise, we need to learn the solution.
		unsigned char dir = select_turn(found_left, found_straight, found_right);

		// Make the turn indicated by the path.
		turn(dir);

		// Store the intersection in the path variable.
		path[path_length] = dir;
		path_length ++;

		// You should check to make sure that the path_length does not
		// exceed the bounds of the array.  We'll ignore that in this
		// example.

		// Simplify the learned path.
		simplify_path();

		// Display the path on the LCD.
		display_path();
	}
}

bool caminho_certo(unsigned char esq, unsigned char frente, unsigned char dir, char caminho) {
	bool retorno = true;

	if(caminho == 'R') {
		if(dir == 0) {
			return false;
		}
	}
	else if(caminho == 'S') {
		if(frente == 0) {
			return false;
		}
	}
	else if(caminho == 'L') {
		if(esq == 0) {
			return false;
		}
	}

	return retorno; 
}

/* Faz o caminho memorizado, mas testa se ele ja nao mudou*/
void resolve_e_reaprende() {

	while(1) {
		// Beep to show that we finished the maze.
		set_motors(0,0);
		play(">>a32");

		// Wait for the user to press a button, while displaying
		// the solution.
		while(!button_is_pressed(BUTTON_B))
		{
			if(get_ms() % 2000 < 1000)
			{
				clear();
				print("Solved!");
				lcd_goto_xy(0,1);
				print("Press B");
			}
			else
				display_path();
			delay_ms(30);
		}
		while(button_is_pressed(BUTTON_B));

		delay_ms(1000);

		/* Comecaa resolver e verificar se o local esta certo */
		int i = 0;
		while(i  != path_length) {

			/* Acha o proximo bloco */
			follow_segment();

			/* Fica com os sensores em cima da linha */
			set_motors(50,50);
			delay_ms(50);

			/* Testa os sensores */
			unsigned char found_left=0;
			unsigned char found_straight=0;
			unsigned char found_right=0;

			unsigned int sensors[5];
			read_line(sensors,IR_EMITTERS_ON);

			if(sensors[0] > 100)
				found_left = 1;
			if(sensors[4] > 100)
				found_right = 1;

			/* Fica com as rodas em cima da linha */
			set_motors(40,40);
			delay_ms(200);

			// Check for a straight exit.
			read_line(sensors,IR_EMITTERS_ON);
			if(sensors[1] > 200 || sensors[2] > 200 ||	 sensors[3] > 200)
				found_straight = 1;

			/* Ele descobre se a saida saiu do lugar */
			if(sensors[1] > 600 && sensors[2] > 600 && sensors[3] > 600)
				break;
			}

			/* Checa se a saida corresponde com a esperada */
			if(caminho_certo(found_left, found_straight, found_right, path[i])) {
				/* Se estiver tudo bem, soh vai */
				turn(path[i]);

				/* Vai guardando ateh descobrir orientacao */
				troca_orientacao(path[i]);
			}
			/* Se o caminho estiver errado, decora o percurso anterior e acha nova saida */
			else {
				set_motors(0,0); //provisorio
				while(1);
			}

			i++;
		}
	}
	


}

int main() {


	/* Inicializa o 3pi */
	inicializa();

	/* Inicializa a posicao inicial do algoritmo */
	inicializa_posicao(&posicao_inicial, NULL);
	posicao_atual = &posicao_inicial;

	inicializa_mapa();

	/* Começa o algoritmo */
	resolve_e_aprende();


	resolve_e_reaprende();

	//o algoritmo nunca deve chegar ateh aqui

	return 0;
}

