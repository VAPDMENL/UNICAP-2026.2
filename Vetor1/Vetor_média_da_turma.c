#include <stdio.h>

int main (void){
    float nota1[30], nota2[30], media[30], soma=0.0, media_da_turma;
    int i;
    for (i = 0; i < 30; i++)
    {
        printf("\nDigite a nota 1 do aluno %d: \n", i+1);
        scanf("%f", &nota1[i]);
        printf("\nDigite a nota 2 do aluno %d: \n", i+1);
        scanf("%f", &nota2[i]);
        media[i]=(nota1[i]*2+3*nota2[i])/5;
        soma = soma+media[i];
    }
    media_da_turma= soma/30;
    
    printf("\n=====NOTAS=====\n");
    printf("\nMedia da turma: %.2f\n", media_da_turma);
    printf("\n=====Notas e médias dos alunos acima da média %.2f=====\n", media_da_turma);
    for (i=0; i< 30; i++) {
        if (media[i]>media_da_turma)
            printf("\n\033[33mAluno :\033[0m %d \033[33mNota 1:\033[0m %.2f \033[33mNota 2:\033[0m %.2f \033[33mMédia:\033[0m %.2f\n", i+1, nota1[i], nota2[i], media[i]);
    
    }
    return 0;
}
