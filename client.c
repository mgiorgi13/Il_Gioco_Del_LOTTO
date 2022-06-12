/*client.c*/
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

char city[11][10] = {"Bari", "Cagliari", "Firenze", "Genova", "Milano", "Napoli", "Palermo", "Roma", "Torino", "Venezia", "Nazionale"};


/*****************************FUNZIONI DI UTILITA'*****************************

Divide un messaggio di un buffer in ingresso in una matrice di char ogniuna contente 
una parola che è stata individuata grazie al separatore del, dato in ingresso.
Input : msg buffer di lettura,
    mmsg matrice nel quale verrà ricopiato il messaggio diviso in base a del,
    del delimitatore con il quale splitto la stringa letta,
    pos puntatore ad un intero che specifica il numero di parole copiate 
*/
 
void message_split(char* msg, char mmsg[][BUFFER_SIZE], char* del, int* pos){
    char *token;
	/*ottengo il primo token*/
    token = strtok(msg, del);

/*  scorro tra i token*/
    while( token != NULL) {
        strncpy(mmsg[*pos], token, strlen(token));
        *pos = *pos + 1;
        token = strtok(NULL, del);
    }
    strncpy(mmsg[*pos], "\0", 2);
}

/*Funzione che si occupa di stampare a video la sita dei comandi complessiva*/

void commands(){
    char cmds[1024] = {"1) !help <comando> --> mostra i dettagli di un comando\n"
            "2) !signup <username> <password> --> crea un nuovo utente\n"
            "3) !login <username> <password> --> autentica un utente\n"
            "4) !invia_giocata g --> invia una giocata g al server\n"
            "5) !vedi_giocate tipo --> visualizza le giocate precedenti dove tipo = {0,1} e permette di visualizzare le giocate passate ‘0’ oppure le giocate attive ‘1’ (ancora non estratte)\n"
            "6) !vedi_estrazione <n> <ruota> --> mostra i numeri delle ultime n estrazioni sulla ruota specificata\n"
            "7) !vedi_vincite --> mostra tutte le vincite del client\n"     
            "8) !esci --> termina il client\n"
            };
    printf("*****************************GIOCO DEL LOTTO *****************************\n"
            "Sono disponibili i seguenti comandi:\n\n%s",cmds);
}

/*Funzione che si occupa di stampare a video la descrizione del comando c*/

void command(int c){
    switch(c){
        case(1):printf("!signup <username> <password>\n"
                "Registra un nuovo utente caratterizzato da username e password,\n"
                "se lo username inserito è gia stato registrato allora il server\n"
                "informa il client ed attende fino a quando non viene inserito\n"
                "uno username valido.\n");
                break;
        case(2):printf("!login <username> <password>\n"
                "Permette di fare l'accesso al server tramite\n"
                "username e password solo in caso in cui il client sia stato\n"
                "gia registrato tramite la procedura \"signup\".\n"
                "Se le credenziali inserite(username e password) non sono valide\n"
                "il client ha la possibilita' di reinserirle ancora due volte,\n"
                "dopo le quali il suo indirizzo ip verrà bloccato per 30 minuti\n"
                "prima di poter comunicare nuovamente con il server.\n");
                break;
        case(3):printf("!invia_giocata –r nomi_ruote –n numeri_giocati –i soldi_puntati\n"
                "Permette di inviare una giocata al server specificando con\n"
                "-r le ruote sulle quali scommettere\n"
                "un massimo di 11 ruote tra le quali:Bari, Cagliari,\n"
                "Firenze, Genova, Milano, Napoli, Palermo, Roma, Torino, Venezia, Nazionale,\n"
                "se invece si indica tutte la scommessa viene fatta su tutte le ruote\n"
                "-n permette di specificare i numeri sui quali si scommette (da 1 a 90)\n"
                "-i permette di specificare l'importo (non necessariamente intero)\n"
                "relativo a: estratto, ambo, terna, quaterna, cinquina.\n"
                "Non si può scommettere su un tipo di giocata se i numeri scommessi\n"
                "sono insufficienti per realizzarla (es:-r Roma -n 1 2 3 -i 0 0 0 4)\n");
                break;
        case(4):printf("!vedi_giocate tipo\n"
                "Permette di vedere le giocate effettuate dal client,\n"
                "tipo è 0 o 1\n"
                "con 0 vengono visualizzate le giocate relative\n"
                "a estrazioni gia' effettuate\n"
                "con 1 vengono visualizzate le giocate relative\n"
                "a estrazioni attive, in attesa della prossima estrazione.\n");
                break;
        case(5):printf("!vedi_estrazione n ruota\n"
                "Permette di visualizzare le n-ultime estrazioni effettuate,\n"
                "se ruota non viene specificato vengono visualizzate tutte le\n"
                "11 ruote, diversamente solo la ruota indicata.\n");
                break;
        case(6):printf("!vedi_vincite\n"
                "Permette di vedere tutte le vincite del client,\n"
                "la relativa estrazione in cui sono state realizzate\n"
                "e un consuntivo per tipologia di giocata.\n");
                break;
        case(7):printf("!esci\n"
                "Permette al client di disconnettersi dal server.\n");
                break;
        default:printf("Comando richiesto non disponibile.\n");
    }
}

/*Le due stringhe comparate sono uguali solo se sono della stessa dimensione e 
 * risultano identiche
 */

bool exactstrcmp(char* s1, char* s2){
    if(strncmp(s1,s2,strlen(s1)) == 0 && strncmp(s1,s2,strlen(s2)) == 0)
        return true;
    return false;
}

/*Funzione che controlla se il carattere c è una lettera dell'alfabeto*/
bool isachar(char c){
    if((c >= 'a' && c <= 'z')
            ||
            (c >= 'A' && c <= 'Z'))
       return true;
    return false;
}

/*Funzione che controlla se il carattere c è un numero*/

bool isanumber(char c){
    if(c >= '0' && c <= '9')
       return true;
    return false;
}

/*Funzione che controlla se il carattere c è una citta del gioco del lotto*/

bool isacity(char* c){
    int i;
    for(i = 0; i < 11; i++){
        if(exactstrcmp(city[i], c))
            return true;
    }
    return false;
}

/*Funzione che si occupa di controllare la sintassi dei comandi per decidere se inviare o meno il messaggio al server 
Input: messaggio inserito da tastiera
Output: true se la sintassi è corretta false in caso contrario*/

bool check_message(char* mes){
    char copy[BUFFER_SIZE];
    char split[BUFFER_SIZE][BUFFER_SIZE];
    char space[2] = " ";
    int slen = 0, i = 0, j = 0;
    int nnbet = 0, nimport = 0;
    bool correct = false;
    
    memset(copy,0,BUFFER_SIZE);
    memset(split,0,BUFFER_SIZE*BUFFER_SIZE);
    
    strncpy(copy,mes,BUFFER_SIZE);
    copy[strlen(copy)-1] = '\0';
    message_split(copy,split,space,&slen);
    
    if(exactstrcmp(split[0],"!help")){
        correct = true;
        if(exactstrcmp(split[1],"\0"))
		/*allora è stato richiesto il comando !help senza paramentro*/
            commands();
        else{ printf("**********************************************************\n");
			/*leggo il parametro e in base a quello stampo la descrizione del comando*/
            if(exactstrcmp(split[1],"signup")){
                command(1);
            } else if(exactstrcmp(split[1],"login")){
                command(2);
            } else if(exactstrcmp(split[1],"invia_giocata")){
                command(3);
            } else if(exactstrcmp(split[1],"vedi_giocate")){
                command(4);
            } else if(exactstrcmp(split[1],"vedi_estrazione")){
                command(5);
            } else if(exactstrcmp(split[1],"vedi_vincite")){
                command(6);
            } else if(exactstrcmp(split[1],"esci")){
                command(7); 
            } else {
                command(8);
            } 
        printf("**********************************************************\n");
        }    
    }else if(exactstrcmp(split[0],"!signup") || exactstrcmp(split[0],"!login")){
		/*per essere valido basta che ci sia un nome utente e una password (numeri o caratteri)*/
        if(split[1][0] != '\0' && split[2][0] != '\0' && split[3][0] == '\0'){
            for(j = 0; j < strlen(split[1]); j++){
                if(!isanumber(split[1][j]) && !isachar(split[1][j]))
                    goto ret;
            }
            for(j = 0; j < strlen(split[2]); j++){
                if(!isanumber(split[2][j]) && !isachar(split[2][j]))
                    goto ret;
            }
            correct = true;
        }
    }else if(exactstrcmp(split[0],"!invia_giocata")){
        if(!exactstrcmp(split[1],"-r"))
            goto ret;
        i = 2;
        while(!exactstrcmp(split[i],"-n")){
            if(i >= slen || (!isacity(split[i]) && !exactstrcmp(split[i],"tutte")))
                goto ret;
            i++;
        }
        /*-n*/
        i++;
        while(!exactstrcmp(split[i],"-i")){
            nnbet++;
            if(atoi(split[i]) < 1 || atoi(split[i]) > 90)
                goto ret;
            if(nnbet > 10 || i >= slen)
                goto ret;
            for(j = 0; j < strlen(split[i]); j++)
                if(!isanumber(split[i][j]))
                    goto ret;
            i++;
        }
        /*-i*/
        i++;
        while(i < slen){
            nimport++;
            if(nimport > 5)
                goto ret;
            for(j = 0; j < strlen(split[i]); j++)
                if(!isanumber(split[i][j]) && split[i][j] != '.')
                    goto ret;
            i++;
        }
        if(nnbet < nimport)
            goto ret;
        correct = true;
    }else if(exactstrcmp(split[0],"!vedi_giocate")){
        if(split[1][0] == '0' || split[1][0] == '1')
            correct = true;
    }else if(exactstrcmp(split[0],"!vedi_estrazione")){
        if(split[1][0] == '\0')
            goto ret;
        for(j = 0; j < strlen(split[1]); j++){
            if(!isanumber(split[1][j]))
                goto ret;
        }
        if(split[2][0] != '\0'){
            if(!isacity(split[2]))
                goto ret;
        }
        correct = true;
    }else if(exactstrcmp(split[0],"!vedi_vincite")){
        if(split[1][0] == '\0')
            correct = true;
    }else if(exactstrcmp(split[0],"!esci")){
        correct = true;
    }
    
ret:
    if(!correct)
        printf("Errore : formato comando non corretto, prova !help per vedere la sintassi dei comandi\n");
    return correct;
}  

int main(int argc, char* argv[]){
    int ret, sd, slen = 0;
    uint32_t lmsg;
    long int len;
    struct sockaddr_in srv_addr;
    char buffer[BUFFER_SIZE], buffer_copy[BUFFER_SIZE], space[2] = " ", session_id[11];
    char split[BUFFER_SIZE][BUFFER_SIZE];
    char * buffer_recv;
 
    /*gestione parametri in ingresso*/

    switch(argc){
        case 1: printf("Errore inserire indirizzo e porta del server\n");
                exit(-1);        
        case 2: printf("Errore inserire porta del server\n");
                exit(-1);
    }

    /* Creazione socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    /* Creazione indirizzo del server */
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);
    
    memset(session_id,0,11);
    
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    
    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(-1);
    }
    else
        commands();
    
    while(1){    
		/*Attendo input da tastiera significativo*/
        len = 0;
        while(len == 0){
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
            
			/*scarto il carattere \n*/
            len = strlen(buffer) -1; 
            if(check_message(buffer))
                break;
            else
                len = 0;
        }
    
        strncpy(buffer_copy,buffer,BUFFER_SIZE);
		/*splitto il messaggio inserito da tastiera*/
        memset(split, 0, BUFFER_SIZE*BUFFER_SIZE);
        slen = 0;
        message_split(buffer_copy,split,space,&slen);
                
		/*controllo se è il comando help*/
        if(exactstrcmp(split[0],"!help\n") || exactstrcmp(split[0],"!help")){       
        }else {/*comando diverso da !help*/
            /*preparo il buffer da inviare */
            if(session_id[0] != '\0'){
                strncpy(buffer_copy, session_id,BUFFER_SIZE);
                strcat(buffer_copy, " ");
                strncat(buffer_copy,buffer,BUFFER_SIZE);
                strncpy(buffer,buffer_copy,BUFFER_SIZE);
                len = strlen(buffer) - 1;
            }
            /*invio al server la quantità di dati*/        
            lmsg = htonl(len);
            ret = send(sd, (void*) &lmsg, sizeof(uint32_t), 0);
            ret = send(sd, (void*) buffer, len, 0);
            if(ret < 0){
                perror("Errore in fase di invio: \n");
                continue;
            }
            
            /*attendo dimensione del messaggio*/
            ret = recv(sd, (void*)&lmsg, sizeof(uint32_t), 0);
            len = ntohl(lmsg); /*Rinconverto in formato host*/
            /* Alloco il buffer per la ricezione del messaggio */
            buffer_recv = (char*)malloc(len);
            if(buffer_recv == NULL){
            	printf("Errore messaggio troppo grande\n");
            	//finisco il dialogo con il server ignorando il messaggio che mi invia
            	ret = recv(sd, (void*)buffer, len, 0);
            	if(ret < 0)
                	perror("Errore in fase di ricezione: \n");
                continue;
            }
            memset(buffer_recv,0,len);
            /* Attendo la risposta*/
            ret = recv(sd, (void*)buffer_recv, len, 0);
            if(ret < 0){
                perror("Errore in fase di ricezione: \n");
                continue;
            }
            /*ho chiamato la login e non mi sono ancora loggato*/
            if(exactstrcmp(split[0],"!login") && 
                    strncmp(buffer_recv,"Errore:",strlen("Errore:")) && 
                    session_id[0] == '\0'){
                strncpy(session_id,buffer_recv,11);
                strcpy(buffer,"Login effettuato correttamente");
                printf("%s\n", buffer);
            }else{
                printf("%s\n", buffer_recv);
            }
            
            if(exactstrcmp(buffer_recv,"!esci") || 
                exactstrcmp(buffer_recv,"Errore: numero di tentativi terminato, ritenta tra 30 minuti") ||
                    strncmp(buffer_recv,"Il tuo IP risulta ancora bannato", strlen("Il tuo IP risulta ancora bannato")) == 0){
                /*dealloco il buffer*/
                memset(buffer_recv,0,len);
                free(buffer_recv);
                break;    
            }
            /*dealloco il buffer*/
            memset(buffer_recv,0,len);
            free(buffer_recv);
       }
    }
    
    close(sd);
return 0;
        
}
