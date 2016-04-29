/* Algoritimo para resolução de labirinto usando Aprendizado de Máquina 
* Proposto por Claude Shannon em seu exerimento do Theseus 
* Simularemos com o 3pi como sendo Theseus 
* O Robô percorrerá o labirinto salvando seus erros e acertos, ao final, quando colocado novamente ele já saberá o menor cmainho para saida 
* Autor: Leonardo Alves de Melo */

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

void setup() {

	/* Inicializa a posicao inicial */
	inicializa_posicao(&posicao_inicial, NULL);
	posicao_atual = &posicao_inicial;

	inicializa_mapa();



}

void loop() {

	testa_saida();
}