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
#include "follow-segment.h"

/* Sera printado na tela LCD ao iniciar o programa*/
const char welcome_line1[] PROGMEM = " GER";
const char welcome_line2[] PROGMEM = "Shannon";
const char demo_name_line1[] PROGMEM = "Maze";
const char demo_name_line2[] PROGMEM = "solver";

/* Musicas do robo */
const char welcome[] PROGMEM = ">g32>>c32";
const char go[] PROGMEM = "L16 cdegreg4";

#define NULL 0

/* Definimos a direcao do robo */
#define NORTE 'n'
#define LESTE 'l'
#define SUL 's'
#define OESTE 'o'

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
#define LARGURA 11
#define ALTURA 11
#define TAM_MAPA LARGURA * ALTURA

int ponte;

/* guardamos o caminho em um vetor de caracter */
char path[TAM_MAPA] = "";

/* Guarda as antigas posicoes do caminho para no caso de ele mudar */
char antigo_path[TAM_MAPA] = "";

/* Cada local descoberto aumentamos o tamanho do caminho */
unsigned char path_length = 0;

/* Vetor 2D que guarda a posicao do robo no mapa e/ou a posicao da saida */
typedef struct Mapa {
	int x;
	int y;
} Mapa;

/* Neste struct está o percurso certo para sair do labirinto antes dele mudar */
typedef struct Percurso {	
	char orientacao;
	unsigned char dir; 
	Mapa posicao;
} Percurso;

/* É o que o robo se lembra */
Percurso percurso[TAM_MAPA/2];

/* Local onde o robo esta no mapa */
Mapa local_robo;

/* Variavel que guarda a orientacao do 3pi */
char orientacao = ORIENTACAO_INICIAL;

/* guarda o tamanho para o novo percurso */
int tam_percurso_memorizado = 0;

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

/* Configuracoes inicial do local do robo e da saida */
void inicializa_mapa() {

	local_robo.x = X_ROBO;
	local_robo.y = Y_ROBO;
}

void display_path()
{
	// Set the last character of the path to a 0 so that the print()
	// function can find the end of the string.  This is how strings
	// are normally terminated in C.
	path[path_length] = 0;

	clear();
	print(path);

	if(path_length > 8)
	{
		lcd_goto_xy(0,1);
		print(path+8);
	}
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
		else if(nova_orientacao == 'B') {
			orientacao = SUL;
		}
	}
	else if(orientacao == SUL) {

		if(nova_orientacao == 'R') {
			orientacao = OESTE;
		}
		else if(nova_orientacao == 'L') {
			orientacao = LESTE;
		}
		else if(nova_orientacao == 'B') {
			orientacao = NORTE;
		}
	} 
	else if(orientacao == LESTE) {

		if(nova_orientacao == 'R') {
			orientacao = SUL;
		}
		else if(nova_orientacao == 'L') {
			orientacao = NORTE;
		}
		else if(nova_orientacao == 'B') {
			orientacao = OESTE;
		}
	}
	else if(orientacao == OESTE) {

		if(nova_orientacao == 'R') {
			orientacao = NORTE;
		}
		else if(nova_orientacao == 'L') {
			orientacao = SUL;
		}
		else if(nova_orientacao == 'B') {
			orientacao = LESTE;
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

/* Se a saida nao corresponder ao caminho */
int caminho_certo(unsigned char esq, unsigned char frente, unsigned char dir, char caminho) {
	int retorno = 1;

	if(caminho == 'R') {
		if(dir == 0) {
			return 0;
		}
	}
	else if(caminho == 'S') {
		if(frente == 0) {
			return 0;
		}
	}
	else if(caminho == 'L') {
		if(esq == 0) {
			return 0;
		}
	}

	return retorno; 
}

/* Escreve na Struct que guarda o caminho */
void acrescenta_caminho(int x, int y, int i) {	

	if(tam_percurso_memorizado == 0) {
		percurso[tam_percurso_memorizado].orientacao = orientacao;
		percurso[tam_percurso_memorizado].dir = path[i];
		percurso[tam_percurso_memorizado].posicao.x = x;	
		percurso[tam_percurso_memorizado].posicao.y = y;
	}
	else {
		percurso[tam_percurso_memorizado].orientacao = orientacao;
		percurso[tam_percurso_memorizado].dir = path[i];
		percurso[tam_percurso_memorizado].posicao.x = percurso[tam_percurso_memorizado - 1].posicao.x + x;	
		percurso[tam_percurso_memorizado].posicao.y = percurso[tam_percurso_memorizado - 1].posicao.y + y;
	}
	

	/* Tem mais um para memorizar */
	tam_percurso_memorizado++;

}

/* Faz uma lista ligada das posicoes que ele passaria a partir da posicao que mudou */
void guarda_caminho_anterior(int pos) {

	int i;
	char orientacao_antiga = orientacao;

	for(i = pos; i < path_length; i++) {
		if(path[i] == 'S') {
			if(orientacao == NORTE) {
				acrescenta_caminho(0,1, i);
			}
			else if(orientacao == LESTE) {
				acrescenta_caminho(1,0, i);
			}
			else if(orientacao == OESTE) {
				acrescenta_caminho(-1,0, i);
			}
			else if(orientacao == SUL) {
				acrescenta_caminho(0,-1, i);
			}
		}
		else if(path[i] == 'R') {
			if(orientacao == NORTE) {
				acrescenta_caminho(1,0, i);
			}
			else if(orientacao == LESTE) {
				acrescenta_caminho(0,-1, i);
			}
			else if(orientacao == OESTE) {
				acrescenta_caminho(0,1, i);
			}
			else if(orientacao == SUL) {
				acrescenta_caminho(-1,0, i);
			}
		}
		else if(path[i] == 'L') {
			if(orientacao == NORTE) {
				acrescenta_caminho(-1,0, i);
			}
			else if(orientacao == LESTE) {
				acrescenta_caminho(0,1, i);
			}
			else if(orientacao == OESTE) {
				acrescenta_caminho(0,-1, i);
			}
			else if(orientacao == SUL) {
				acrescenta_caminho(1,0, i);
			}
		}

		troca_orientacao(path[i]);

		/* Guarda o antigo caminho para ser usado se reconhecido depois */
		antigo_path[i] = path[i];
	}

	orientacao = orientacao_antiga;
}

/* Gira para a orientacao desejada */
void gira(char nova_orientacao) {

	if(orientacao == nova_orientacao) {
		return;
	}

	if(nova_orientacao == NORTE) {
		if(orientacao == LESTE) {
			turn('L');
			path[path_length++] = 'L';
		}
		else if(orientacao == SUL) {
			turn('B');
			path[path_length++] = 'B';
		}
		else if(orientacao == OESTE) {
			turn('R');
			path[path_length++] = 'R';
		}
	}
	else if(nova_orientacao == LESTE) {
		if(orientacao == SUL) {
			turn('L');
			path[path_length++] = 'L';
		}
		else if(orientacao == OESTE) {
			turn('B');
			path[path_length++] = 'B';
		}
		else if(orientacao == NORTE) {
			turn('R');
			path[path_length++] = 'R';
		}
	}
	else if(nova_orientacao == SUL) {
		if(orientacao == OESTE) {
			turn('L');
			path[path_length++] = 'L';
		}
		else if(orientacao == NORTE) {
			turn('B');
			path[path_length++] = 'B';
		}
		else if(orientacao == LESTE) {
			turn('R');
			path[path_length++] = 'R';
		}
	}
	else if(nova_orientacao == OESTE) {
		if(orientacao == NORTE) {
			turn('L');
			path[path_length++] = 'L';
		}
		else if(orientacao == LESTE) {
			turn('B');
			path[path_length++] = 'B';
		}
		else if(orientacao == SUL) {
			turn('R');
			path[path_length++] = 'R';
		}
	}

	orientacao = nova_orientacao;
}

/* Escreve no vetor o caminho antigo que sabemos que pode estar certo */
void atualiza_path(int pos) {

	int i;

	ponte = path_length;

	for(i = pos + 2 ; i < tam_percurso_memorizado; i++) {

		path[path_length++] = percurso[i].dir;
	}

	display_path();

}

/* Checa todo vetor de posicoes ja passadas para saber se ja passou por ali */
int testa_se_ja_passou(char dir) {
	
	int i;

	for(i = 0; i < tam_percurso_memorizado; i++) {
		/* Se estamos em um local que ele ja passou */
		if(percurso[i].posicao.x == local_robo.x && percurso[i].posicao.y == local_robo.y) {

			turn(dir);
			troca_orientacao(dir);

			follow_segment();

			/* Fica com os sensores em cima da linha */
			set_motors(50,50);
			delay_ms(50);

			/* Fica com as rodas em cima da linha */
			set_motors(40,40);
			delay_ms(200);

			path[path_length++] = dir;

			/* Gira para ficar como estava na posicao em que passou por ali */
			gira(percurso[i].orientacao);


			atualiza_path(i);
			return 1;
		}
	}


	return 0;

}

/* Atualiza a posicao atual do robo e testa se ele ja passou por ali */
int atualiza_e_checa(char dir) {

	if(dir == 'S') {
		if(orientacao == NORTE) {
			local_robo.y += 1;
		}
		else if(orientacao == LESTE) {
			local_robo.x += 1;
		}
		else if(orientacao == OESTE) {
			local_robo.x += -1;
		}
		else if(orientacao == SUL) {
			local_robo.y += -1;
		}
	}
	else if(dir == 'R') {
		if(orientacao == NORTE) {
			local_robo.x += 1;
		}
		else if(orientacao == LESTE) {
			local_robo.y += -1;
		}
		else if(orientacao == OESTE) {
			local_robo.y += 1;
		}
		else if(orientacao == SUL) {
			local_robo.x += -1;
		}
	}
	else if(dir == 'L') {
		if(orientacao == NORTE) {
			local_robo.x += -1;
		}
		else if(orientacao == LESTE) {
			local_robo.y += 1;
		}
		else if(orientacao == OESTE) {
			local_robo.y += -1;
		}
		else if(orientacao == SUL) {
			local_robo.x += 1;
		}
	}
	else if(dir == 'B') {
		if(orientacao == NORTE) {
			local_robo.y += -1;
		}
		else if(orientacao == LESTE) {
			local_robo.x += -1;
		}
		else if(orientacao == OESTE) {
			local_robo.x += 1;
		}
		else if(orientacao == SUL) {
			local_robo.y += 1;
		}
	}

	return testa_se_ja_passou(dir);

}

void printa_local() {
	/* Printa o local do robo */
	clear();
	print("(");

	char str1[2];

	if(local_robo.x < 0) {
		print("-");

		str1[0] = (-local_robo.x) + '0';

	}
	else {
		str1[0] = local_robo.x + '0';
	}
	str1[1] = 0;
	print(str1);

	char str2[2];
	print(",");
	if(local_robo.y < 0) {
		print("-");

		str2[0] = (-local_robo.y) + '0';

	}
	else {
		str2[0] = local_robo.y + '0';
	}
	str2[1] = 0;
	print(str2);
	print(")");
}

void printa_posicoes_futuras() {

	int i;

	set_motors(0,0);

	while(1){
		for(i = 0; i < tam_percurso_memorizado; i++) {
			/* Printa o local do robo */
			clear();
			print("(");

			char str1[2];

			if(percurso[i].posicao.x < 0) {
				print("-");

				str1[0] = (-percurso[i].posicao.x) + '0';

			}
			else {
				str1[0] = percurso[i].posicao.x + '0';
			}
			str1[1] = 0;
			print(str1);

			char str2[2];
			print(",");
			if(percurso[i].posicao.y < 0) {
				print("-");

				str2[0] = (-percurso[i].posicao.y) + '0';

			}
			else {
				str2[0] = percurso[i].posicao.y + '0';
			}
			str2[1] = 0;
			print(str2);
			print(")");

			delay(1000);
		}
	}

	

}

/* Faz o caminho memorizado, mas testa se ele ja nao mudou*/
void resolve_e_reaprende() {

	while(1) {

		orientacao = NORTE;

		// Beep to show that we finished the maze.
		set_motors(0,0);
		play(">>a32");

		// Wait for the user to press a button, while displaying
		// the solution.
		while(!button_is_pressed(BUTTON_B))
		{
			if(get_ms() % 2000 < 1000)
			{
				//clear();
				//print("Resolvido!");
				//lcd_goto_xy(0,1);
				//print("Aperte B");
			}
			else
				//display_path();
			delay_ms(30);
		}
		while(button_is_pressed(BUTTON_B));

		delay_ms(1000);

		clear();

		/* Comecaa resolver e verificar se o local esta certo */
		int i = 0;
		while(1) {

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

			/* Checa se a saida corresponde com a esperada */
			if(caminho_certo(found_left, found_straight, found_right, path[i])) {
				
				/* Se estiver tudo bem, soh vai */
				turn(path[i]);

				/* Vai guardando ateh descobrir orientacao */
				troca_orientacao(path[i]);

				char string[2];

				string [0] = orientacao;
				string[1] = 0;
				/* Escreve na tela a orientacao atual */				
				print(string);

				i++;
			}
			/* Se o caminho estiver errado, decora o percurso anterior e acha nova saida */
			else {

				clear();
				//print("MUDOU!");				

				/* Guarda o antigo caminho ate onde parou */
				guarda_caminho_anterior(i);

				//printa_posicoes_futuras();

				/* Atualiza o novo tamanho */
				path_length = i;

				/* Termina de ver o local para saber onde ir */
				unsigned char dir = select_turn(found_left, found_straight, found_right);				
				turn(dir);

				path[path_length] = dir;

				atualiza_e_checa(dir);
				troca_orientacao(dir);


				path_length++;

				// Simplify the learned path.
				simplify_path();

				//printa_local();
				char string[2];

				string [0] = orientacao;
				string[1] = 0;
				/* Escreve na tela a orientacao atual */				
				print(string);


				/* A partir de agora, comeca o algoritmo de resolucao do labirinto, mas sempre vendo se ja passou por um local */
				while(1) {

					

					// FIRST MAIN LOOP BODY  
					follow_segment();					

					// Drive straight a bit.  This helps us in case we entered the
					// intersection at an angle.
					// Note that we are slowing down - this prevents the robot
					// from tipping forward too much.
					set_motors(50,50);
					delay_ms(50);

					
					found_left=0;
					found_straight=0;
					found_right=0;

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
					dir = select_turn(found_left, found_straight, found_right);

					/* Se o local que ele chegou agora faz parte do caminho seguinte ao que ele estava antes, ele sabe resolver */
					if(atualiza_e_checa(dir)) {

						i = ponte;

						/* Vai para a proxima posicao do vetor ja atualizado */
							


						break;
					}

					// Make the turn indicated by the path.
					turn(dir);
					troca_orientacao(dir);

					string [0] = orientacao;
					string[1] = 0;
					/* Escreve na tela a orientacao atual */				
					print(string);

					// Store the intersection in the path variable.
					path[path_length] = dir;
					path_length++;

					// You should check to make sure that the path_length does not
					// exceed the bounds of the array.  We'll ignore that in this
					// example.

					// Simplify the learned path.
					simplify_path();

					// Display the path on the LCD.
					//display_path();

					printa_local();
						

					


				}
			}
				
				

		}

			
		
	}
}

int main() {


	/* Inicializa o 3pi */
	inicializa();


	inicializa_mapa();

	/* Começa o algoritmo */
	resolve_e_aprende();


	resolve_e_reaprende();

	//o algoritmo nunca deve chegar ateh aqui

	return 0;
}

