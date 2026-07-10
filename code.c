#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Protótipo das funções para já avisar o compilador que elas existem
void tela_inicial();
int tela_menu();
void tela_jogar();
void tela_config();
void tela_instruc();
void tela_ranking();
void zerar_ranking();

//Agora sim as funções do programa
void tela_inicial() {
    printf("\n\n\n\nBem vindo(a) ao jogo <BUS TERMINAL>!\n\n\n\n");
    printf("Informe seu nickname: ");
}

int tela_menu() {
    system("clear");
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
    system("clear");
}

void tela_config() {
    int opcao_config;
    system("clear");
    printf("*** CONFIGURACOES ***\n\n");
    printf("1 - Zerar ranking\n");
    printf("2 - Voltar ao menu principal\n\n");
    printf("Digite a opcao desejada: ");
    scanf("%d", &opcao_config);
    switch(opcao_config) {
        case 1:
            zerar_ranking();
        break;
        case 2:
            tela_menu();
        break;
        default:
            printf("Opcao invalida! Tecle <enter> para continuar: ");
            getchar(); 
            getchar();
    }
}

void tela_instruc() {
    system("clear");
    printf("\n\n\n\nINSTRUCOES SOBRE O JOGO <BUS TERMINAL>:\n\n");
    printf("(Instrucoes sobre como jogar)\n\n");
    printf("Tecle <enter> para prosseguir: ");
    getchar(); 
    getchar();
}

void tela_ranking() {
    system("clear");
}

void zerar_ranking() {
    system("clear");
}

int main () {
    int opcao;
    char nickname[100];
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