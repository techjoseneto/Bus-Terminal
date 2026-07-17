#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//protótipo das funções para já avisar o compilador que elas existem
void tela_inicial();
int tela_menu();
void tela_jogar();
void tela_config();
void tela_instruc();
void tela_ranking();
void zerar_ranking();
void gravar_ranking(char nick[], int pont);
void desenho_onibus(int fase, int onibus, char motorista, char assentos[], char fila_espera[], int num_linhas, char regua[]);

//comando para limpar a tela de acordo com o sistema operacional usado
#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

//variáveis globais
FILE* fp; //ponteiro para o arquivo
char nickname[20],arquivo[20];
int pontuacao=0;
char grid[10][15]; //matriz que guarda o grid dos passageiros da fase
char bus[5][22]; //matriz que guarda o desenho do ônibus
char sequencia_onibus[10]; //string que guarda a ordem dos ônibus na fase
struct dadosjogador {
    char nickname[20];
    int pontuacao;
};
#define RANKING_ARQUIVO "ranking.bin"

//agora sim as funções do programa
void tela_inicial() {
    printf("\n\n\n\nBem vindo(a) ao jogo <BUS TERMINAL>!\n\n\n\n");
    printf("Informe seu nickname: ");
}

int tela_menu() {
    system(CLEAR);
    int opcao_menu;
    printf("*** JOGO <BUS TERMINAL> ***\n\n");
    printf("1 - Jogar\n");
    printf("2 - Configuracoes\n");
    printf("3 - Instrucoes\n");
    printf("4 - Ranking\n");
    printf("5 - Sair\n\n");
    printf("Digite a opcao desejada: ");
    scanf("%d", &opcao_menu);
    return opcao_menu;
}

void tela_jogar() {
    system(CLEAR);
    int qtd_onibus, num_linhas, campo_reservado, num_colunas, i, j, k;
    char regua[20];
    char final_fase[5];
    int fase_atual = 1;
    char motorista = ' ';
    int sair_forcado = 0;
    pontuacao = 0; // Zera a pontuacao a cada nova partida
    fp = fopen("entrada.txt", "r"); //"r" abre o arquivo no modo leitura
    if(fp==NULL) {
        printf("Erro! Arquivo nao encontrado!\n");
        printf("Tecle <enter> para voltar: ");
        getchar();
        getchar();
        return;
    }
    int continuar_jogo=1; // Flag para controlar o laço de leitura de fases

    //lendo as informações do arquivo
    while (continuar_jogo==1 && fscanf(fp, "%d %d %d %s", &qtd_onibus, &num_linhas, &campo_reservado, regua)==4) {
        // A largura real do grid é o tamanho da regua (ex: "1234567890" = 10 colunas),
        // e não o terceiro campo do arquivo, que não corresponde à quantidade de colunas exibidas
        num_colunas = (int)strlen(regua);

        // Limpa o buffer até o final da linha para que o fgets funcione corretamente lendo espaços
        int ch;
        while ((ch = fgetc(fp)) != '\n' && ch != EOF);

        for(i=0; i<num_linhas; i++) {
            // Buffer temporário maior que o grid para garantir que o \n seja
            // sempre consumido, mesmo quando a linha tem exatamente 14 colunas
            char linha_buffer[64];
            fgets(linha_buffer, sizeof(linha_buffer), fp);
            linha_buffer[strcspn(linha_buffer, "\r\n")] = '\0'; // Remove o \n do final da string
            strncpy(grid[i], linha_buffer, 14);
            grid[i][14] = '\0';
        }
        fscanf(fp, "%s", sequencia_onibus);
        fscanf(fp, "%s", final_fase);
        int onibus_atual=1;
        char fila_espera[6] = "_____"; // Fila inicia com underlines
        char assentos[3] = {' ', ' ', ' '};
        int fase_concluida=0;
        int game_over=0;
        sair_forcado=0; // Reseta a flag a cada nova fase

        while (fase_concluida==0 && game_over==0 && sair_forcado==0) {
            system(CLEAR);
            motorista = sequencia_onibus[onibus_atual-1];

            // Verifica se alguém da fila pode entrar no ônibus atual antes de pedir o próximo input
            for (j=0; j<5; j++) {
                if (fila_espera[j] == motorista) {
                    for (k=0; k<3; k++) {
                        if (assentos[k] == ' ') {
                            assentos[k] = fila_espera[j];
                            fila_espera[j] = '_'; // Volta a ser underline quando remove da fila
                            break;
                        }
                    }
                }
            }

            // Verifica se o ônibus encheu após o processamento automático
            if (assentos[0] != ' ' && assentos[1] != ' ' && assentos[2] != ' ') {
                onibus_atual++;
                pontuacao += 15; // Ganha 15 pontos a cada onibus enchido
                for (j=0; j<3; j++) {
                    assentos[j] = ' '; // Limpa assentos
                }
                if (onibus_atual > qtd_onibus) {
                    fase_concluida = 1;
                    continue; // Pula o resto do loop para sair
                }
            }

            desenho_onibus(fase_atual, onibus_atual, motorista, assentos, fila_espera, num_linhas, regua);

            int linha_escolhida, coluna_escolhida;
            printf("\nInforme a linha e coluna (0 0 para sair): ");
            scanf("%d %d", &linha_escolhida, &coluna_escolhida);

            // Verifica a desistência do jogador
            if (linha_escolhida == 0 && coluna_escolhida == 0) {
                sair_forcado = 1;
            }
            // Validação simples para evitar erro de acesso fora da matriz
            else if (linha_escolhida >= 1 && linha_escolhida <= num_linhas &&
                     coluna_escolhida >= 1 && coluna_escolhida <= num_colunas) {

                int r = linha_escolhida - 1;
                int c = coluna_escolhida - 1;
                char passageiro = grid[r][c];
                int lin;

                // Verifica se tem passageiro (célula vazia é '_' ou ' ', não é passageiro real)
                if (passageiro != '_' && passageiro != ' ' && passageiro != '\0' && passageiro != '\n' && passageiro != '\r') {

                    // Verifica se o passageiro tem caminho livre até a linha 0.
                    // Para cada linha acima, o "corredor" de saída é a propria coluna (c)
                    // ou a coluna imediatamente a direita (c+1): basta uma das duas estar
                    // livre em cada linha para o passageiro conseguir passar por ali.
                    int livre = 1; // Assume livre até encontrar uma linha totalmente bloqueada
                    if (r > 0) {
                        for (lin=r-1; lin>=0; lin--) {
                            int celula_livre = ((size_t)c >= strlen(grid[lin]) ||
                                                 grid[lin][c] == '_' || grid[lin][c] == ' ');

                            if (celula_livre==0 && c + 1 < num_colunas) {
                                celula_livre = ((size_t)(c+1) >= strlen(grid[lin]) ||
                                                 grid[lin][c+1] == '_' || grid[lin][c+1] == ' ');
                            }
                            if (celula_livre==0) {
                                livre = 0; // Esta linha não tem abertura: caminho bloqueado
                                break;
                            }
                        }
                    }

                    if (livre == 0) {
                        // Existe passageiro na posição, mas ele está bloqueado
                        printf("\nElemento bloqueado! Tecle <enter> para continuar: ");
                        while (getchar() != '\n'); // Limpa o buffer do scanf
                        getchar();
                    } else {
                        if (passageiro == motorista) {
                            // Encontra o primeiro assento vazio
                            for (j=0; j<3; j++) {
                                if (assentos[j] == ' ') {
                                    assentos[j] = passageiro;
                                    grid[r][c] = ' '; // Remove do grid
                                    break;
                                }
                            }
                        } else {
                            // Coloca na fila de espera
                            for (j=0; j<5; j++) {
                                if (fila_espera[j] == '_') {
                                    fila_espera[j] = passageiro;
                                    grid[r][c] = ' ';
                                    break;
                                }
                            }

                            // Verifica se a fila lotou
                            int fila_cheia = 1;
                            for (j=0; j<5; j++) {
                                if (fila_espera[j] == '_') {
                                    fila_cheia = 0;
                                    break;
                                }
                            }
                            if (fila_cheia == 1) {
                                game_over = 1; // Fila cheia: jogador perde
                            }
                        }

                        // Verifica se o ônibus encheu (3 passageiros) após o movimento
                        if (assentos[0] != ' ' && assentos[1] != ' ' && assentos[2] != ' ') {
                            onibus_atual++;
                            pontuacao += 15; // Ganha 15 pontos a cada onibus enchido
                            // Limpa assentos para o próximo ônibus
                            for (j =0; j<3; j++) {
                                assentos[j] = ' ';
                            }
                            // Verifica se acabou os ônibus da fase
                            if (onibus_atual>qtd_onibus) {
                                fase_concluida=1;
                            }
                        }
                    }
                } else {
                    // Posição dentro da matriz, mas sem passageiro ali
                    printf("\nPosicao invalida! Tecle <enter> para continuar: ");
                    while (getchar() != '\n');
                    getchar();
                }
            } else {
                // Entrada fora dos limites da matriz
                printf("\nPosicao invalida! Tecle <enter> para continuar: ");
                while (getchar() != '\n'); // Limpa o buffer do scanf
                getchar();
            }
        }

        if (game_over==1) {
            system(CLEAR);
            desenho_onibus(fase_atual, onibus_atual, motorista, assentos, fila_espera, num_linhas, regua);
            printf("\n*** GAME OVER ***\n");
            printf("Fila lotada. VOCE PERDEU!\n");
            printf("Pontuacao final: %i\n", pontuacao);
            gravar_ranking(nickname, pontuacao);
            continuar_jogo=0; // Encerra o jogo
        } else if (sair_forcado==1) {
            gravar_ranking(nickname, pontuacao);
            continuar_jogo=0; // Volta para o menu se o jogador desistiu
        } else if (final_fase[0]=='U') {
            pontuacao += 100; // Ganha 100 pontos de bonus por vencer a fase
            system(CLEAR);
            printf("*** JOGO <BUS TERMINAL> ***\n");
            printf("Pontuacao:%i\n\n", pontuacao);
            printf("*****************************************************\n");
            printf("**            PARABENS!!! VOCE VENCEU!!!            **\n");
            printf("**                                                   **\n");
            printf("**                Tecle <enter>                     **\n");
            printf("*****************************************************\n");
            gravar_ranking(nickname, pontuacao);
            continuar_jogo=0; // Encerra o jogo com vitória
        } else {
            pontuacao += 100; // Ganha 100 pontos de bonus por vencer a fase
            char continuar_resposta;
            system(CLEAR);
            printf("*** JOGO <BUS TERMINAL> ***\n");
            printf("Pontuacao:%i\n\n", pontuacao);
            printf("*****************************************************\n");
            printf("**       MUITO BEM, VOCE FINALIZOU A FASE!          **\n");
            printf("**                                                   **\n");
            printf("**       Continuar para a proxima fase? (S/N)       **\n");
            printf("*****************************************************\n");
            scanf(" %c", &continuar_resposta);
            if (continuar_resposta == 'N' || continuar_resposta == 'n') {
                gravar_ranking(nickname, pontuacao);
                continuar_jogo=0; // Jogador optou por nao continuar
            } else {
                fase_atual++; // Passa para a próxima fase
            }
        }
    }

    fclose(fp);
    if (continuar_jogo == 0 && sair_forcado == 0) {
        printf("Tecle <enter> para voltar ao menu: ");
        while (getchar() != '\n');
        getchar();
    }
}

void tela_config() {
    int opcao_config;
    system(CLEAR);
    printf("*** CONFIGURACOES ***\n\n");
    printf("1 - Zerar ranking\n");
    printf("2 - Modo dificuldade (ainda nao foi implementado)\n");
    printf("3 - Editor de fases (ainda nao foi implementado)\n");
    printf("4 - Voltar ao menu principal\n\n");
    printf("Digite a opcao desejada: ");
    scanf("%d", &opcao_config);
    switch(opcao_config) {
        case 1:
            zerar_ranking();
        break;
        case 2:
        break;
        case 3:
        break;
        case 4:
        break;
        default:
            printf("Opcao invalida! Tecle <enter> para continuar: ");
            getchar(); 
            getchar();
    }
}

void tela_instruc() {
    system(CLEAR);
    printf("\n\nINSTRUCOES SOBRE O JOGO <BUS TERMINAL>:\n\n");
    printf("BUS TERMINAL eh jogado pelo terminal do computador e o objetivo eh\n");
    printf("embarcar todos os passageiros do grid no seu respectivo onibus. Cada\n");
    printf("onibus so aceita passageiros do seu proprio tipo (simbolo).\n\n");
    printf("Para jogar, informe a linha e a coluna do passageiro que quer\n");
    printf("movimentar. Posicoes invalidas, vazias ou passageiros bloqueados nos\n");
    printf("quatro lados nao podem ser movidos. Quando o simbolo bate com o onibus\n");
    printf("da vez, o passageiro embarca direto; quando nao bate, ele vai pra fila\n");
    printf("de espera ate o onibus certo aparecer. Ao completar um onibus, o\n");
    printf("proximo da sequencia entra e os passageiros compativeis na fila\n");
    printf("embarcam sozinhos.\n\n");
    printf("Cada onibus completado vale 15 pontos, e a cada fase completada ha\n");
    printf("mais 100 de bonus. Se a fila de espera lotar, voce perde o jogo. Se\n");
    printf("voce zerar o grid, voce escolhe se quer seguir para a proxima fase.\n\n");
    printf("Tecle <enter> para prosseguir: ");
    getchar();
    getchar();
}

void gravar_ranking(char nick[], int pont) {
    FILE* fr = fopen(RANKING_ARQUIVO, "ab"); // "ab" cria o arquivo se nao existir, e adiciona ao final
    if (fr == NULL) {
        return;
    }
    struct dadosjogador registro;
    strncpy(registro.nickname, nick, 19);
    registro.nickname[19] = '\0';
    registro.pontuacao = pont;
    fwrite(&registro, sizeof(struct dadosjogador), 1, fr);
    fclose(fr);
}

void tela_ranking() {
    system(CLEAR);
    printf("*** JOGO <BUS TERMINAL> ***\n\n");
    printf("RANKING:\n\n");

    FILE* fr = fopen(RANKING_ARQUIVO, "rb");
    if (fr == NULL) {
        printf("(ranking vazio)\n");
    } else {
        struct dadosjogador ranking[1000];
        int total = 0;
        while (total < 1000 && fread(&ranking[total], sizeof(struct dadosjogador), 1, fr) == 1) {
            total++;
        }
        fclose(fr);

        //bubblesort pra ordenar a pontuação
        int a, b;
        for (a = 0; a < total - 1; a++) {
            for (b = 0; b < total - 1 - a; b++) {
                if (ranking[b].pontuacao < ranking[b + 1].pontuacao) {
                    struct dadosjogador temp = ranking[b];
                    ranking[b] = ranking[b + 1];
                    ranking[b + 1] = temp;
                }
            }
        }

        if (total == 0) {
            printf("(ranking vazio)\n");
        } else {
            for (a = 0; a < total; a++) {
                printf("%d %s\n", ranking[a].pontuacao, ranking[a].nickname);
            }
        }
    }

    printf("\nTecle <enter> para continuar: ");
    while (getchar() != '\n');
    getchar();
}

void zerar_ranking() {
    system(CLEAR);
    printf("*** JOGO <BUS TERMINAL> ***\n\n");
    char resposta;
    printf("Confirma reinicializar o ranking? (S/N) ");
    scanf(" %c", &resposta);
    if (resposta == 'S' || resposta == 's') {
        FILE* fr = fopen(RANKING_ARQUIVO, "wb"); // "wb" recria o arquivo vazio
        if (fr != NULL) {
            fclose(fr);
        }
        printf("Ranking zerado! Tecle <enter> para continuar: ");
    } else {
        printf("Operacao cancelada! Tecle <enter> para continuar: ");
    }
    while (getchar() != '\n');
    getchar();
}

void desenho_onibus (int fase, int onibus, char motorista, char assentos[], char fila_espera[], int num_linhas, char regua[]){
    int i;
    printf("*** JOGO <BUS TERMINAL> ***\n");
    printf("*** FASE %i *** ONIBUS %i *** PONTUACAO %i ***\n\n", fase, onibus, pontuacao);
    printf("+----o-------o----+\n");
    printf("|   -     -   -   |-+\n");
    printf("|  |%c  |%c  |%c    |%c |\n", assentos[0], assentos[1], assentos[2], motorista);
    printf("|   -    -    -   |-+\n");
    printf("+----o---==--o----+\n\n");
    printf("    ");
    for (i=0; i<5; i++) {
        printf("%c ", fila_espera[i]=='_' ? ' ' : fila_espera[i]);
    }
    printf("\n    ");
    for (i=0; i<5; i++) {
        printf("_ ");
    }
    printf("\n\n");
    for (i=0; i<num_linhas; i++) {
        printf("%d %s\n", i + 1, grid[i]);
    }
    printf("  %s\n", regua);
}
int main () {
    int opcao;
    tela_inicial();
    scanf("%[^\n]", nickname);
    do {
        opcao=tela_menu();
        switch(opcao) {
            case 1:
                tela_jogar();
            break;
            case 2:
                tela_config();
            break;
            case 3:
                tela_instruc();
            break;
            case 4:
                tela_ranking();
            break;
            case 5:
            break;
            default:
                printf("Opcao invalida! Tecle <enter> para continuar: ");
                getchar(); 
                getchar();
            break;
        }
    } while(opcao!=5);
    return 0;
}