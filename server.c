/*server.c*/
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define BAN_TIME 30
#define SLEEP_TIME 60
#define ESTRATTO 11.23
#define AMBO 250
#define TERNA 4500
#define QUATERNA 120000
#define CINQUINA 6000000


char city[11][10] = {"Bari", "Cagliari", "Firenze", "Genova", "Milano", "Napoli", "Palermo", "Roma", "Torino", "Venezia", "Nazionale"};


/******************************FUNZIONI DI UTILITA'*****************************

 Estrae i numeri del LOTTO e li inserisce nel file delle estrazioni insieme al
 timestamp relativo all'estrazione
 input : orario di estrazione
 output : numero positivo se tutto ok numero negativo in caso di errore */
int extraction(time_t ext_time) {
    /*apertura del file in append -> se il file esiste scrivo in fondo
     senno creo il file*/
    FILE* extfile;
    char support[BUFFER_SIZE], number_support[3];
    int extracted_number[5];
    int ret, wheel = 0, count, random_number, i;
    bool exists = false;

    /* ricopio l'orario di estrazione nel buffer di appoggio*/
    snprintf(support, BUFFER_SIZE, "%ld \n", ext_time);

    while (wheel < 11) {
        strncat(support, city[wheel], BUFFER_SIZE);
        strncat(support, " ", 2);
        /*ripulisco l'array da valori precendenti*/
        memset(&extracted_number, 0, sizeof (int)*5);
        srand((unsigned) (time(&ext_time) + wheel));
        count = 0;

        /*genero 5 numeri casuali per ruota*/
        while (count < 5) {
            /* genero un numero casuale tra 1 e 90*/
            random_number = rand() % 90 + 1;
            exists = false;

            /*estraggo un numero solo una volta*/
            for (i = 0; i < count; ++i) {
                if (extracted_number[i] == random_number) {
                    exists = true;
                    break;
                }
            }
            if (!exists) { /*se non esiste allora è stato generato correttamente*/
                extracted_number[count] = random_number;
                count++;
            }
        }
        /*ricopio nel buffer di appoggio i numeri estratti*/
        count = 0;
        while (count < 4) {
            snprintf(number_support, 3, "%d", extracted_number[count]);
            strncat(support, number_support, BUFFER_SIZE);
            strncat(support, " ", 2);
            count++;
        }
        snprintf(number_support, 3, "%d", extracted_number[4]);
        strncat(support, number_support, BUFFER_SIZE);
        strncat(support, " \n", 3);
        wheel++;
    }

    /*apertura del file estrazioni*/
    extfile = fopen("estrazioni.txt", "a");
    if (extfile == NULL)
        return -1;

    /*ricopio nel file il buffer di appoggio contente tutte le informazioni
     riguardanti l'estrazione effettuata*/
    ret = fprintf(extfile, "%s", support);

    /*chiusura del file estrazioni*/
    fclose(extfile);
    return ret;
}

/*Le due stringhe comparate sono uguali solo se sono della stessa dimensione e 
 * risultano identiche
 */

bool exactstrcmp(char* s1, char* s2) {
    if (strncmp(s1, s2, strlen(s1)) == 0 && strncmp(s1, s2, strlen(s2)) == 0)
        return true;
    return false;
}

/*Divide un messaggio di un buffer in ingresso in una matrice di char ogniuna contente
una parola che è stata individuata grazie al separatore del, dato in ingresso.
Input : msg buffer di lettura,
    mmsg matrice nel quale verrà ricopiato il messaggio diviso in base a del,
    del delimitatore con il quale splitto la stringa letta,
    pos puntatore ad un intero che specifica il numero di parole copiate
 */

void message_split(char* msg, char mmsg[][BUFFER_SIZE], char* del, int* pos) {
    char *token;
    *pos = 0;
    /*ottengo il primo token*/
    token = strtok(msg, del);

    /*scorro tra i token*/
    while (token != NULL) {
        strncpy(mmsg[*pos], token, strlen(token));
        *pos = *pos + 1;
        token = strtok(NULL, del);
    }
    
    strncpy(mmsg[*pos], "\0", 2);
}

/* Genera una stringa di caratteri casuali di dimensione 10
Input : array, buffer di destinazione per la stringa creata
 */
void session_id_generator(char array[]) {
    static int j = 0;
    int i;
    srand((unsigned) time(NULL) + j++);
    for (i = 0; i < 10; i++)
        array[i] = (char) ((rand() % 26) + 65);
    array[i] = '\0'; 
}

/*Controlla se l'ip inserito può ristabilire la connessione con il
 server ovvero se ip non compare tra ip_bloccati.txt oppure se compare ma
 è passato un tempo sufficiente per ristabilire la connessione
 Input: ip del cliente che sta cercando di fare l'accesso
        time ovvero l'ora attuale da confrontre con quella inserita
        in ip_bloccati.txt
 Output: >= BAN_TIME || < 0 -> l'ip può ristabilire la connessione
         = -1 -> il file non può essere aperto
         >0 && < BAN_TIME -> l'ip non può ristabilire la connessione */

int check_ip(unsigned short int ip, time_t time) {
    FILE* fd;
    char sip[64], scanner[64];
    double diff = -1;

    /*trasformo ip in char*/
    snprintf(sip, 64, "%hu", ip);

    /*apro il file ip e cerco l'ip passato come parametro*/
    fd = fopen("ip_bloccati.txt", "r");

    if (fd == NULL) {
        return -1;
    }

    while (fscanf(fd, "%s", scanner) != EOF) { /*leggo una riga alla volta*/

        if (exactstrcmp(scanner, sip)) {/*ip trovato*/
            fscanf(fd, "%s", scanner);
            diff = difftime(time, atol(scanner));
        } else
            fscanf(fd, "%s", scanner);
        
        memset(scanner,0,64);
    }

    /*chiusura del file*/
    fclose(fd);
    
    if(diff == -1)
        return diff;
    return diff / 60;

}
/*Funzione che calcola il fattoriale di un numero
Input: intero n (numero del quale calcolare il fattoriale)
Output: long int result risultato del fattoriale */
long int fact(int n) {
    int i;
    long int result = 1;
    if (n == 0)
        return 1;
    for (i = n; i > 0; i--) {
        result = result * i;
    }
    return result;
}

/*Funzione che arrotonda alla seconda cifra decimale il numero dato in ingresso
Input: float n 
Output: float result (n arrotondato)*/
float roundf2(float n){
    float res;
    res = n*100;
    res = roundf(n);
    res = n/100;
    return res;
}

/*Funzione che ricerca il numero bet tra l'array di numeri ext
Input: int bet numero da ricercare, int* ext array di 5 numeri nel quale si cerca il numero  
Output: bool : true se bet è uno dei numeri di ext, diversamente è false*/
bool search(int bet, int ext[5]) {
    int i;
    for (i = 0; i < 5; i++) {
        if (bet == ext[i])
            return true;
    }
    return false;
}

/* Funzione che si occupa di calcolare la vincita di un determinato utente e una determinata scommessa 
per ogni estrazione si cerca quella relativa a la data scommessa fatta (quella la cui differenza di tempo tra
ora estrazione e ora scommessa risulta positiva e piu piccola possibile) dopo di che si analizza la scommessa e 
i numeri estratti per le varie ruote, si calcolano le vittorie parziali relative ad ogni ruota e ad ogni tipo di giocata
effettuato, secondo le regole del gioco del lotto.
Input: char* user nome dell'utente, char* bet scommessa effettuata dall'utente, time_t* bet_time tempo dell'ultima estrazione(permette 
di mettere insime quelle scommesse che sono avvenute prima di una determinata estrazione)
Output: bool : true in caso in cui sia stata trovata l'estrazione relativa a quella scommessa, false in caso contrario*/
bool calcola_vincita(char* user, char* bet, time_t* bet_time) {
    char copy[BUFFER_SIZE], winresult[BUFFER_SIZE];
    char split[BUFFER_SIZE][BUFFER_SIZE];
    char space[2] = " ";
    int extnumber[5], numbers[10];
    long int possibility, winning_comb;
    bool found = false, wheels[11], first = true;
    float amount[5];
    float win = 0;
    int i = 0, j = 0, pos = 0, nwheels = 0, nbetnumber = 0, nwinnumber = 0;
    FILE* extfile, *winfile, *played;
    time_t extraction_time;
    struct tm *info;
    /*pulizia*/
    memset(copy, 0, BUFFER_SIZE);
    memset(split, 0, BUFFER_SIZE * BUFFER_SIZE);
    memset(winresult, 0, BUFFER_SIZE);
    memset(wheels, 0, 11 * sizeof (bool));
    memset(numbers, 0, 10 * sizeof (int));
    memset(amount, 0, 5 * sizeof (float));

    strncpy(copy, bet, BUFFER_SIZE);
    message_split(copy, split, space, &pos);

    /*analizzo la scommessa*/
    i = 2; /*salto il timestamp e il comando -r*/
    if(exactstrcmp(split[i],"tutte")){
        memset(wheels, true, 11* sizeof(bool));
        nwheels = 11;
        i++;
    }else{
        while (!exactstrcmp(split[i], "-n")) { /*leggo le ruote*/
            j = 0;
            while (strncmp(split[i], city[j], strlen(split[i])) != 0 && j < 11) {
                j++;
            }
            /*ho trovato la ruota*/
            wheels[j] = true;
            nwheels++;

            i++;
        }
    }
    i++;
    j = 0;
    while (!exactstrcmp(split[i], "-i")) {/*leggo i numeri giocati*/
        numbers[j++] = atoi(split[i++]);
    }
    i++;
    nbetnumber = j;
    j = 0;
    while (i < pos)/*leggo gli importi giocati*/
        amount[j++] = atof(split[i++]);

    memset(copy, 0, BUFFER_SIZE);
    strncpy(copy, user, BUFFER_SIZE);
    strcat(copy, "/vincite.txt");
    /*apro il file relativo alle vincite*/
    winfile = fopen(copy, "a");
    if (winfile == NULL) {
        perror("Errore in calcola_vincita ");
        return false;
    }
    memset(copy, 0, BUFFER_SIZE);
    strncpy(copy, user, BUFFER_SIZE);
    strcat(copy, "/giocate_effettuate.txt");
    /*apro il file relativo alle giocate effettuate */
    played = fopen(copy, "a");
    if (played == NULL) {
        perror("Errore in calcola_vincita ");
        return false;
    }
    /*cerco l'estrazione relativa a quella scommessa*/
    extfile = fopen("estrazioni.txt", "r");
    if (extfile == NULL) {
        perror("Errore in calcola_vincita ");
        return false;
    }
    memset(copy, 0, BUFFER_SIZE);
    /*cerco l'estrazione d'interesse*/
    while (fscanf(extfile, "%s", copy) != EOF) {
        if (difftime(atol(copy), atol(split[0])) >= 0) { /* ho trovato l'estrazione che mi interessa*/
            extraction_time = atol(copy);
            info = localtime(&extraction_time);
            i = 0;
            found = true;
            while (i < 11) {
                memset(copy, 0, BUFFER_SIZE);
                fscanf(extfile, "%s", copy); /*ottengo la città*/
                if (wheels[i]) {/*per tutte le ruote sulle quali ho scommesso*/
                    /*copio la ruota e i numeri giocati*/
                    strncat(winresult, city[i], BUFFER_SIZE);
                    strcat(winresult, " ");
                    nwinnumber = 0;
                    for (j = 0; j < nbetnumber; j++) {
                        snprintf(copy, BUFFER_SIZE, "%d ", numbers[j]);
                        strncat(winresult, copy, BUFFER_SIZE);
                    }
                    strcat(winresult, " >>  ");
                    /*recupero i numeri estratti per quella ruota*/
                    for (j = 0; j < 5; j++) {
                        memset(copy, 0, BUFFER_SIZE);
                        fscanf(extfile, "%s", copy);
                        extnumber[j] = atoi(copy);
                    }

                    /*calcolo i numeri indovinati*/
                    for (j = 0; j < nbetnumber; j++) {
                        if (search(numbers[j], extnumber))/*se il numero è presente */
                            nwinnumber++;
                    }

                    /*calcolo le vincite relative alle possibili combinazioni*/
                    if (amount[4] != 0) {/* ho scommesso cinquine*/

                        strcat(winresult, "Cinquina ");
                        if (nwinnumber == 5 && amount[4] == 0) {/*ho indovinato la cinquina*/
                            possibility = fact(nbetnumber) / (120 * fact(5 - nbetnumber));
                            /*calcolo vincita per le cinquine*/
                            win = (1 * CINQUINA * amount[4]) / (nwheels * possibility);
                        } else {
                            win = 0;
                        }
                        /*arrotondo alla seconda cifra decimale */
                        win = roundf2(win);
                        
                        snprintf(copy, BUFFER_SIZE, "%f", win);
                        strncat(winresult, copy, BUFFER_SIZE);
                        strcat(winresult, "  ");
                    }
                    if (amount[3] != 0) {/* ho scommesso quaterne*/

                        strcat(winresult, "Quaterna ");
                        possibility = fact(nbetnumber) / (24 * fact(4 - nbetnumber));
                        /* se ho indovinato piu di 4 numeri ma non ho fatto giocate di tipo superiore*/
                        if (nwinnumber > 4 && amount[4] == 0) {
                            winning_comb = fact(nwinnumber) / (24 * fact(4 - nwinnumber));
                            /*calcolo vincita per le quaterne */
                            win = (winning_comb * QUATERNA * amount[3]) / (nwheels * possibility);
                        } else if (nwinnumber == 4)/* ho indovinato la quaterna*/
                            win = (1 * QUATERNA * amount[3]) / (nwheels * possibility);
                        else
                            win = 0;
                        
                        /*arrotondo alla seconda cifra decimale */
                        win = roundf2(win);
                        
                        snprintf(copy, BUFFER_SIZE, "%f", win);
                        strncat(winresult, copy, BUFFER_SIZE);
                        strcat(winresult, "  ");

                    }
                    if (amount[2] != 0) {/* ho scommesso terne*/

                        strcat(winresult, "Terna ");
                        possibility = fact(nbetnumber) / (6 * fact(3 - nbetnumber));
                        /* se ho indovinato piu di 3 numeri ma non ho fatto giocate di tipo superiore*/
                        if (nwinnumber > 3 && amount[4] == 0 && amount[3] == 0) {
                            winning_comb = fact(nwinnumber) / (6 * fact(3 - nwinnumber));
                            /*calcolo vincita per le terne */
                            win = (nwinnumber * TERNA * amount[2]) / (nwheels * possibility);
                        } else if (nwinnumber == 3)/* ho indovinato la terna*/
                            win = (1 * TERNA * amount[2]) / (nwheels * possibility);
                        else
                            win = 0;
                        
                        /*arrotondo alla seconda cifra decimale */
                        win = roundf2(win);
                        
                        snprintf(copy, BUFFER_SIZE, "%f", win);
                        strncat(winresult, copy, BUFFER_SIZE);
                        strcat(winresult, "  ");

                    }
                    if (amount[1] != 0) {/* ho scommesso ambi*/

                        strcat(winresult, "Ambo ");
                        possibility = fact(nbetnumber) / (2 * fact(2 - nbetnumber));
                        /*se ho indovinato piu di 2 numeri ma non ho fatto giocate di tipo superiore*/
                        if (nwinnumber > 2 && amount[4] == 0 && amount[3] == 0 && amount[2] == 0) {
                            winning_comb = fact(nwinnumber) / (2 * fact(2 - nwinnumber));
                            /*calcolo vincita per gli ambi */
                            win = (nwinnumber * AMBO * amount[1]) / (nwheels * possibility);
                        } else if (nwinnumber == 2)/* ho indovinato l'ambo*/
                            win = (1 * AMBO * amount[1]) / (nwheels * possibility);
                        else
                            win = 0;
                        
                        /*arrotondo alla seconda cifra decimale */
                        win = roundf2(win);
                        
                        snprintf(copy, BUFFER_SIZE, "%f", win);
                        strncat(winresult, copy, BUFFER_SIZE);
                        strcat(winresult, "  ");

                    }
                    if (amount[0] != 0) {/*ho scommesso estratti*/

                        strcat(winresult, "Estratto ");
                        possibility = nbetnumber;
                        /*se ho indovinato piu di 1 numero ma non ho fatto giocate di tipo superiore*/
                        if (nwinnumber > 1 && amount[4] == 0 && amount[3] == 0 && amount[2] == 0 && amount[1] == 0) {
                            winning_comb = fact(nwinnumber) / (1 * fact(1 - nwinnumber));
                            /*calcolo vincita per gli estratti */
                            win = (nwinnumber * ESTRATTO * amount[0]) / (nwheels * possibility);
                        } else if (nwinnumber == 1)/* ho indovinato l'estratto*/
                            win = (1 * ESTRATTO * amount[0]) / (nwheels * possibility);
                        else
                            win = 0;
                        
                        /*arrotondo alla seconda cifra decimale */
                        win = roundf2(win);
                        
                        snprintf(copy, BUFFER_SIZE, "%f", win);
                        strncat(winresult, copy, BUFFER_SIZE);
                        strcat(winresult, "  ");

                    }
                    strncat(winresult, " \n", 3);
                    /*controllo se quella estrazione è gia stata inserita */
                    if (extraction_time != *bet_time) {/* se gli orari sono diversi*/
                        *bet_time = extraction_time;
                        memset(copy, 0, BUFFER_SIZE);
                        strftime(copy, BUFFER_SIZE, "***********************************************\nEstrazione del %d-%m-%Y ore %H:%M \n", info);
                        strncat(copy, winresult, BUFFER_SIZE);
                        strncpy(winresult, copy, BUFFER_SIZE);
                    }
                    if(first){
                        /*se è la prima ruota di cui calcolo la vincita allora
                         inserisco anche la giocata tra le giocate effettuate*/
                        first = false;
                        /*inserisco la giocata tra le giocate effettuate*/
                        memset(copy, 0, BUFFER_SIZE);
                        strncpy(copy, split[0], BUFFER_SIZE);
                        for (j = 1; j < pos; j++) {
                            strcat(copy, " ");
                            strncat(copy, split[j], BUFFER_SIZE);
                        }
                        fprintf(played, "%s", copy);
                    }
                    /*inserisco le relative vincite*/
                    fprintf(winfile, "%s", winresult);
                } else {
                    /*mi preparo per leggere la prossima citta*/
                    fgets(copy, BUFFER_SIZE, extfile);
                }
                i++;
                memset(winresult, 0, BUFFER_SIZE);
            }
            /*se ho trovato l'estrazione allora posso uscire dal ciclo*/
            break;
        } else {/*non è l'estrazione corretta*/
            /*salto il carattere di torna a capo*/
            fgets(copy, BUFFER_SIZE, extfile);
            /*salto tutte le ruote*/
            i = 0;
            while (i < 11) {/*salto tutte le citta*/
                memset(copy, 0, BUFFER_SIZE);
                fgets(copy, BUFFER_SIZE, extfile);
                i++;
            }
        }
        memset(copy, 0, BUFFER_SIZE);
    }
    fclose(extfile);
    fclose(winfile);
    fclose(played);
    if (found)/*se ho trovato l'estrazione*/
        return true;
    return false;
}

/*Funzione che calcola la lunghezza di un file e il numero di righe 
Input: char* nome del file del quale calcolare la lunghezza, int* numero di righe del file
Output: long int lunghezza del file*/

long int file_size(char* file_name, int* line_number) {
    long int size = 0;
    char read[BUFFER_SIZE];

    *line_number = 0;
    memset(read, 0, BUFFER_SIZE);

    FILE* fp;
    if ((fp = fopen(file_name, "r")) == NULL) {
        perror("Error file");
        return 0;
    }

    while (fgets(read, BUFFER_SIZE, fp) != NULL) {
        *line_number = *line_number + 1;
        size = size + strlen(read)+1;
        memset(read, 0, BUFFER_SIZE);
    }
    fclose(fp);
    return size;
}

/*Funzione che aggiorna i file utente u (giocate_attive,giocate_effettuate,vincite)
per ogni giocata presente nel file utente giocate_attive viene chiamata la funzione calcola vincita
se questa va a buon fine vuol dire che la giocata_attiva ora è presente tra le giocate effettuate e è stata calcolata 
la relativa vincita nel file vincite, in caso contrario se calcola_vincita ritorna false allora la giocata verrà ricopiata
in un buffer di appoggio per poi essere ricopiata dentro il file giocate_attive
Input:char* u nome dell'utente
Output:true in caso di nessun errore*/
bool aggiorna_file_utente(char* u) {
    char reading_buffer[BUFFER_SIZE];
    char * buffer;
    int nline = 0;
    long int betsize = 0;
    FILE* active_bet;
    time_t bt = time(NULL);
    bool first = true;

    memset(reading_buffer, 0, BUFFER_SIZE);

    strncpy(reading_buffer, u, BUFFER_SIZE);
    strcat(reading_buffer, "/giocate_attive.txt");
    betsize = file_size(reading_buffer, &nline);

    active_bet = fopen(reading_buffer, "r");
    /*apro il file delle giocate attive */
    if (active_bet == NULL) {
        perror("Errore in aggiorna_file_utente ");
        return false;
    }
    buffer = (char*) malloc(betsize);
    if(buffer == NULL){
    	perror("Errore in aggiorna_file_utente ");
    	return false;
    }
    memset(buffer, 0, betsize);
    memset(reading_buffer, 0, BUFFER_SIZE);
    /*per ogni giocata chiamo calcola_vincita*/
    while (fgets(reading_buffer, BUFFER_SIZE, active_bet) != NULL) {
        /*se non ha successo allora vuol dire che la giocata dovrà rimanere tra quelle attive*/
        if (!calcola_vincita(u, reading_buffer, &bt)) {
            if (first) {
                strncpy(buffer, reading_buffer, betsize);
                first = false;
            } else
                strncat(buffer, reading_buffer, betsize);
        }
        memset(reading_buffer, 0, BUFFER_SIZE);
    }
    fclose(active_bet);
    /*ripristino il contenuto di giocate attive*/
    if (first == false || buffer[0] == 0) {
        memset(reading_buffer, 0, BUFFER_SIZE);
        strncpy(reading_buffer, u, strlen(u)+1);
        strcat(reading_buffer, "/giocate_attive.txt");
        active_bet = fopen(reading_buffer, "w");
        /*apro il file delle giocate attive */
        if (active_bet == NULL) {
            perror("Errore in aggiorna_file_utente ");
            return false;
        }
        fprintf(active_bet, "%s", buffer);
        fclose(active_bet);
    }
    return true;
}

/******************************FUNZIONI DI SERVIZIO*****************************

 * Iscrive un utente u con password p se u non esiste ancora, ovvero se non è presente 
 * nel file registro_utenti.txt. Se la registrazione va a buon termine vengono anche
 * creati una cartella con il nome dell'utente e al suo interno 3 file relativi a 
 * giocate_effettuate, giocate e vincite.
 * input: u = username p = password dell'utente che si vuole registrare. (p contiente anche il carattere \n)
 * output:  -2 -> errore in fase di creazione dei file utente
            -1 -> errore in caso di scrittura all'interno del file
 *          0 -> in caso di utente gia registrato 
 *          1 -> in caso di registrazione corretta
 */

int signup(char* u, char* p) {
    char reg[BUFFER_SIZE];
    pid_t pid;
    FILE* r;
    bool exists = false;
    int status;

    /*controllo se esiste il file registro_utenti*/
    r = fopen("registro_utenti.txt", "r");
    if (r == NULL) {/*file inesistente*/
        r = fopen("registro_utenti.txt", "w");
        strncpy(reg, u, strlen(u)+1);
        strcat(reg, " ");
        strncat(reg, p, strlen(p)+1);
        strcat(reg, "\n");
        if (fprintf(r, "%s", reg) < 0)
            return -1;
        fclose(r);
    } else { /* il file esiste, quindi cerco se esiste u*/
        /*preparo il buffer per la lettura*/
        memset(reg, 0, BUFFER_SIZE);
        while (fscanf(r, "%s", reg) != EOF) {
            if (exactstrcmp(reg, u)) {/*utente trovato*/
                exists = true;
            } else/* passo al prossimo utente*/
                fscanf(r, "%s", reg);
            /*preparo il buffer per la prossima lettura*/
            memset(reg, 0, BUFFER_SIZE);
        }
        if (exists)/*se l'utente esiste*/
            return 0;
        else {/*se non esiste inserisco in append la coppia utente e password*/
            fclose(r);
            r = fopen("registro_utenti.txt", "a");
            strncpy(reg, u, strlen(u)+1);
            strcat(reg, " ");
            strncat(reg, p, strlen(p)+1);
            strcat(reg, "\n");
            if (fprintf(r, "%s", reg) < 0)
                return -1;
            fclose(r);
        }
    }

    pid = fork();
    if (pid == 0) {/*crea una cartella di nome u contente i file utente*/
        execl("/bin/mkdir", "mkdir", u, NULL);
        printf("Errore nella creazione della cartella utente %s\n", u);
        exit(1);
    } else {
        /*aspetto la creazione della cartella */
        wait(&status);

        if (WIFEXITED(status)) {/*se la creazione dei file utente è avvenuta correttamente*/
            /*creo i relativi file dell'utente (u_giocate_effettuate,u_giocate,u_vincite)*/
            memset(reg, 0, BUFFER_SIZE);
            strncpy(reg, u, BUFFER_SIZE);
            strcat(reg, "/giocate_effettuate.txt");
            r = fopen(reg, "w");
            fclose(r);
            memset(reg, 0, BUFFER_SIZE);
            strncpy(reg, u, BUFFER_SIZE);
            strcat(reg, "/giocate_attive.txt");
            r = fopen(reg, "w");
            fclose(r);
            memset(reg, 0, BUFFER_SIZE);
            strncpy(reg, u, BUFFER_SIZE);
            strcat(reg, "/vincite.txt");
            r = fopen(reg, "w");
            fclose(r);
        } else {
            return -2;
        }
    }
    return 1;

}

/* Permette ad un cliente di effettuare l'accesso al suo account specificando
 * u e p rispettivamente username e password, se queste sono presenti allora
 * viene generato un session_id tramite il quale deve avvenire la comunicazione 
 * tra client e server successivamente alla login, in caso contrario viene decrementato 
 * il numero di tentativi e viene restituito errore.
 * input: u e p username e password dell'utente che si vuole registrare
 *          si buffer nel quale viene inserito il session id
 *          att intero che contiene il numero di tentativi possibili per l'utente
 *          per provare ad effettuare l'accesso
 * output: -1 -> errore apertura file
 *         -2 -> u e p non trovati nella lista utenti
 *         -3 -> tentativi esauriti
 *          1 -> in caso di successo
 */

int login(char* u, char* p, char* si, int* att) {
    char reg[BUFFER_SIZE];
    FILE* r;
    
    r = fopen("registro_utenti.txt", "r");
    if (r == NULL) {
        perror("Errore durante login ");
        return (-1);
    }

    /*preparo il buffer per la lettura*/
    memset(reg, 0, BUFFER_SIZE);
    memset(si,0,11);
    /*controllo di username e password all'interno di registro_utenti*/
    while (fscanf(r, "%s", reg) != EOF) {
        if (exactstrcmp(reg, u)) {/*utente trovato*/
            /*preparo il buffer per la prossima lettura*/
            memset(reg, 0, BUFFER_SIZE);
            fscanf(r, "%s", reg);
            /*controllo che non sia stato inserito solo il nome e*/
            /*che la password sia corretta */
            if (exactstrcmp(reg, p)) {
                /*se u esiste e p è valida allora genero il session_id*/
                session_id_generator(si);
                break;
            }
        } else/* passo al prossimo utente*/
            fscanf(r, "%s", reg);
        /*preparo il buffer per la prossima lettura*/
        memset(reg, 0, BUFFER_SIZE);
    }
    fclose(r);
    /*aggiorno il numero di tentativi*/
    *att = *att - 1;
    /*tentativi esauriti*/
    if(*att == 0)/*tentativi esauriti*/
        return -3;
    else if(si[0] != '\0')/*session_id generato per l'utente trovato*/
        return 1;
    else/*diversamente la coppia utente e password non è stata trovata*/
        return -2;
}

/* Permette ad un cliente di vedere le proprie giocate attive(1) oppure effettuate(0)
 * input: char* file_name nome del file del quale si vedranno le giocate
 *        char* result buffer che contiene il risultato della funzione
 *        long int resultlen lunghezza massima del buffer di risultato
 * output: -1 -> errore apertura file
 *         -2 -> file "file_name" vuoto
 *          1 -> in caso di successo
 */

int vedi_giocate(char* file_name, char* result, long int resultlen) {
    char read[BUFFER_SIZE];
    char split[BUFFER_SIZE][BUFFER_SIZE];
    char space[2] = " ";
    int count = 1, slen = 0, i = 2, importcount = 1;
    FILE * bet_file;
    bet_file = fopen(file_name, "r");

    if (resultlen == 0)/*file vuoto*/
        return -2;

    memset(read, 0, BUFFER_SIZE);
    memset(split, 0, BUFFER_SIZE * BUFFER_SIZE);
    memset(result, 0, resultlen);

    if (bet_file == NULL) {
        perror("Errore durante vedi_giocate");
        return -1;
    }
    while (fgets(read, BUFFER_SIZE, bet_file) != NULL) {
        /*splitto il messaggio per controllarlo meglio*/
        message_split(read, split, space, &slen);
        /*ricopio le giocate rispettando la formattazione richiesta*/
        if(count == 1)
            snprintf(result, resultlen, "%d", count);
        else{
            snprintf(read, resultlen, "%d", count);
            strncat(result, read, resultlen);
        }
        strcat(result, ") ");
        
        while (!exactstrcmp(split[i], "-n")) {/*ricopio le città*/
            strncat(result, split[i], resultlen);
            strcat(result, " ");
            i++;
        }
        i++; /*salto -n*/
        while (!exactstrcmp(split[i], "-i")) {/*ricopio i numeri*/
            strncat(result, split[i], resultlen);
            strcat(result, " ");
            i++;
        }
        i++; /*salto -i*/
        while (i < slen - 1) {/*ricopio gli importi*/
            if (atoi(split[i]) != 0) {
                switch (importcount++) {
                    case(1):
                        strncat(result, "* ", resultlen);
                        strncat(result, split[i], 5);
                        strcat(result, " estratto ");
                        i++;
                        break;
                    case(2):
                        strncat(result, "* ", resultlen);
                        strncat(result, split[i], 5);
                        strcat(result, " ambo ");
                        i++;
                        break;
                    case(3):
                        strncat(result, "* ", resultlen);
                        strncat(result, split[i], 5);
                        strcat(result, " terna ");
                        i++;
                        break;
                    case(4):
                        strncat(result, "* ", resultlen);
                        strncat(result, split[i], 5);
                        strcat(result, " quaterna ");
                        i++;
                        break;
                    default:
                        strncat(result, "* ", resultlen);
                        strncat(result, split[i], 5);
                        strcat(result, " cinquina ");
                        i++;
                }
            } else {
                importcount++;
            }
        }
        strcat(result, "\n");

        memset(read, 0, BUFFER_SIZE);
        memset(split, 0, BUFFER_SIZE * BUFFER_SIZE);
        count++;
        importcount = 1;
        i = 2;
    }
    /*rimuovo l'ultimo torna a capo in quanto verrà inserito dal client*/
    result[strlen(result)-1] = '\0';
    fclose(bet_file);
    return 1;
}

/* Permette ad un cliente di vedere le ultime n estrazioni del gioco del lotto complessive(tutte le ruote) oppure di una determinata ruota
 * input: char* result buffer che contiene il risultato della funzione
 *        long int resultlen lunghezza massima del buffer di risultato
 *        int nskip numero di estrazioni da saltare durante lo scorrimento del file per poter vedere solo le ultime n
 *        char* wheel eventuale ruota della quale si vogliono vedere le estrazioni, in caso in cui sia vuota il risultato contiene tutte le ruote
 * output: void
 */

void vedi_estrazione(char* result, long int resultlen, int nskip, char* wheel){
    char read[BUFFER_SIZE];
    int i = 0, nwheel = 0;
    FILE * extfile;
    time_t extraction_time;
    struct tm *info;

    memset(result,0,resultlen);
    memset(read,0,BUFFER_SIZE);

    extfile = fopen("estrazioni.txt","r");
    if(extfile == NULL){
        perror("Errore in vedi_estrazione");
        strcpy(result,"Impossibile visualizzare le estrazioni");
        return;
    }
    
    /*cerco il numero di quella ruota*/
    i = 0;
    while(i < 11){
        if(exactstrcmp(wheel,city[i]))
            nwheel = i + 1;
        i++;
    }

    /*salto le estrazioni non richieste */
    while(nskip != 0){
        nskip--;
        i = 0;
        while(i <= 11){/*salto una estrazione*/
            fgets(read, BUFFER_SIZE, extfile);
            i++;
        }
    }
    i = 0;
    memset(read,0,BUFFER_SIZE);
    /*ricopio le estrazioni di interesse*/
    while(fgets(read, BUFFER_SIZE, extfile) != NULL){
        if(i == 0){/*è il timestamp*/
            /*se non è l'ulima riga che contiene il carattere di torna a capo*/
            if(!exactstrcmp(read,"\n")){
                /*rimuovo il carattere di torna a capo*/
                read[strlen(read)-1] = '\0';
                extraction_time = atol(read);
                info = localtime(&extraction_time);
                memset(read,0,BUFFER_SIZE);
                strftime(read, BUFFER_SIZE, "***********************************************\nEstrazione del %d-%m-%Y ore %H:%M \n", info);

                if(result[0] == '\0')/*primo inserimento*/
                    strncpy(result, read, resultlen);    
                else
                    strncat(result, read, resultlen);
            }  
        }else{/*è l'estrazione*/
            /*(se nwheel = 0 allora visualizzo tutte le città)*/
            if(nwheel == 0 || nwheel == i)
                strncat(result,read,resultlen);
        }
        i++;
        memset(read,0,BUFFER_SIZE);
        if(i == 12)
            i = 0;
    }
    /*rimuovo l'ultimo torna a capo*/
    result[strlen(result)-1] = '\0';
    fclose(extfile);
}

/* Permette ad un cliente di vedere le sue vincite recuperandole dal file vincite.txt con un relativo riassunto delle vincite al termine del messaggio 
 * input: char* file_name nome del file dal quale ricavare le vincite
 *        char* result buffer che contiene il risultato della funzione
 *        long int resultlen lunghezza massima del buffer di risultato
 * output: -1 -> errore apertura file
 *          1 -> in caso di successo
 */

int vedi_vincite(char* file_name, char* result, long int resultlen) {
    char read[BUFFER_SIZE];
    char split[BUFFER_SIZE][BUFFER_SIZE];
    char space[2] = " ";
    int slen = 0, i = 0;
    float s_cinquina = 0, s_quaterna = 0, s_terna = 0, s_ambo = 0, s_estratto = 0;
    FILE * victory_file;
    victory_file = fopen(file_name, "r");

    memset(read, 0, BUFFER_SIZE);
    memset(split, 0, BUFFER_SIZE * BUFFER_SIZE);
    memset(result, 0, resultlen);

    if (victory_file == NULL) {
        perror("Errore durante vedi_vincite");
        return -1;
    }

    fgets(read, BUFFER_SIZE, victory_file);
    strncpy(result,read,strlen(read)+1);
    memset(read,0,BUFFER_SIZE);
    
    while(fgets(read, BUFFER_SIZE, victory_file) != NULL){
        /*ricopio la stringa letta nel buffer result*/
        strncat(result,read, strlen(read)+1);
        message_split(read, split, space, &slen);
        if(read[0] == '*' || exactstrcmp(split[0], "Estrazione")){
            memset(read,0,BUFFER_SIZE);
            memset(split,0,BUFFER_SIZE*BUFFER_SIZE);
            continue;
        }else{
            i = 0;
            while(i < slen -1){
                if(exactstrcmp(split[i],"Cinquina")){
                    s_cinquina = s_cinquina + atof(split[++i]);
                    i++;
                }else if(exactstrcmp(split[i],"Quaterna")){
                    s_quaterna = s_quaterna + atof(split[++i]);
                    i++;
                }else if(exactstrcmp(split[i],"Terna")){
                    s_terna = s_terna + atof(split[++i]);
                    i++;
                }else if(exactstrcmp(split[i],"Ambo")){
                    s_ambo = s_ambo + atof(split[++i]);
                    i++;
                }else if(exactstrcmp(split[i],"Estratto")){
                    s_estratto = s_estratto + atof(split[++i]);
                    i++;
                }else
                    i++;
            }
        }
        memset(read,0,BUFFER_SIZE);
        memset(split,0,BUFFER_SIZE*BUFFER_SIZE);
    }

    memset(read,0,BUFFER_SIZE);
    strcat(result, "***********************************************\n\n");
    
    /*arrotondo le vincite alla seconda cifra decimale*/
    
    s_estratto = roundf2(s_estratto);
    s_ambo = roundf2(s_ambo);
    s_terna = roundf2(s_terna);
    s_quaterna = roundf2(s_quaterna);
    s_cinquina = roundf2(s_cinquina);
    
    snprintf(read, BUFFER_SIZE, "Vincite su ESTRATTO: %f", s_estratto);
    strcat(read, "\n");
    strncat(result, read,resultlen);
    memset(read,0,BUFFER_SIZE);

    snprintf(read, BUFFER_SIZE, "Vincite su AMBO: %f", s_ambo);
    strcat(read, "\n");
    strncat(result, read,resultlen);
    memset(read,0,BUFFER_SIZE);

    snprintf(read, BUFFER_SIZE, "Vincite su TERNA: %f", s_terna);
    strcat(read, "\n");
    strncat(result, read,resultlen);
    memset(read,0,BUFFER_SIZE);

    snprintf(read, BUFFER_SIZE, "Vincite su QUATERNA: %f", s_quaterna);
    strcat(read, "\n");
    strncat(result, read,resultlen);
    memset(read,0,BUFFER_SIZE);

    snprintf(read, BUFFER_SIZE, "Vincite su CINQUINA: %f", s_cinquina);
    strncat(result, read,resultlen);
    memset(read,0,BUFFER_SIZE);

    fclose(victory_file);
    return 1;
}

int main(int argc, char* argv[]) {
    /*ritorno, socket di ascolto, socket per il servizio
     periodo esecuzione estrazione
     slen utilizzata per contare il numero di parole presenti in split
     attempts numero tentativi per il login
     i per scorrere il buffer split
     lnumber numero di righe di un file*/
    int ret, sd, new_sd, period, slen = 0, attempts = 3, i = 0, lnumber = 0;
    /*dimensione del file, lunghezza messaggio in formato host*/
    long int file_len = 0, len;
    /*risultatto di check ip*/
    double ciresult = -1;
    /*lunghezza del messaggio in formato network*/
    uint32_t lmsg;
    /*pid per gestire fork: pidr->figlio che si occupa delle estrazioni, pidc-> figlio che si occupa delle connessioni*/
    pid_t pidr, pidc;
    /*struttura del socket del server e del client*/
    struct sockaddr_in my_addr, cl_addr;
    /*buffer per lo scambio di informazioni,
    buffer utilizzato come appoggio,
    buffer contenente il session_id,
    buffer contenete i tentativi di login,
    delimitatore per eseguire lo split del buffer
    betfile contiene il path del file contenente le scommesse 
    dell'utente che ha effettuato il login
    buffer che contiene il nome dell'utenteche si è loggato
    buffer file per la letturada file*/
    char buffer[BUFFER_SIZE], support[BUFFER_SIZE], session_id[11], sattempts[3];
    char space[2] = " ", betfile[BUFFER_SIZE], username[BUFFER_SIZE];
    char * buffer_file;
    /*matrice che contiene il messaggio splittato*/
    char split[BUFFER_SIZE][BUFFER_SIZE];
    /*puntatore a file*/
    FILE* fd;
    /*booleano che specifica se l'invio viene fatto da file*/
    bool send_from_file = false;
    /*pulizia*/
    memset(buffer, 0, BUFFER_SIZE);
    memset(support, 0, BUFFER_SIZE);
    memset(betfile, 0, BUFFER_SIZE);
    memset(username, 0, BUFFER_SIZE);
    memset(session_id, 0, 11);
    memset(sattempts, 0, 3);
    memset(split, 0, BUFFER_SIZE * BUFFER_SIZE);

    /*gestione parametri in ingresso*/
    switch(argc){
        case 1: printf("Errore inserire porta ed eventuale periodo \n");
                exit(-1);
        case 2: period = 5;
                break;
        default : period = atoi(argv[2]);
    }

    /*Padre si occupa delle connessini
    figlio si occupa di eseguire periodicamente le estrazioni */
    pidr = fork();

    if (pidr == 0) {/*figlio che si occupa delle estrazioni*/
        while (1) {
            /*eseguo un'estrazione*/
            ret = extraction(time(NULL));
            if (ret > 0)
                printf(">> Estrazione effettuata\n");
            else
                printf("Errore nella crezione o scrittura del file estrazione.txt\n");
            sleep(period * 60);
        }
    } else if (pidr > 0) {
        /* Creazione socket */
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd > 0)
            printf(">> Socket creato\n");
        else {
            perror("Errore in fase di creazione del Socket: \n");
            exit(-1);
        }

        /* Creazione indirizzo di bind */
        memset(&my_addr, 0, sizeof (my_addr)); /* Pulizia*/
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(atoi(argv[1]));
        my_addr.sin_addr.s_addr = INADDR_ANY;

        ret = bind(sd, (struct sockaddr*) &my_addr, sizeof (my_addr));

        if (ret < 0) {
            perror("Errore in fase di bind: \n");
            exit(-1);
        } else
            printf(">> Socket collegato\n");

        ret = listen(sd, 10);
        printf(">> Socket in ascolto\n");

        while (1) {

            /* Accetto nuove connessioni*/
            while (1) {
                len = sizeof (cl_addr);
                new_sd = accept(sd, (struct sockaddr*) &cl_addr, (unsigned int*)&len);
                ciresult = check_ip(ntohs(cl_addr.sin_addr.s_addr), time(NULL));

                if (new_sd < 0) {
                    perror("Errore in fase di accept: \n");
                    exit(-1);
                } else if (ciresult >= BAN_TIME || ciresult < 0) {
                    printf(">> Connessine accettata\n");
                    memset(session_id, 0, 11);
                    memset(sattempts, 0, 3);
                    memset(betfile, 0, BUFFER_SIZE);
                    attempts = 3;
                    break;
                } else {
                    printf("Connessione dall' IP( %hu ) non concessa\n", ntohs(cl_addr.sin_addr.s_addr));
                    /*pulizia dei buffer*/
                    memset(buffer, 0, BUFFER_SIZE);
                    /*ricevo il messaggio dal client*/
                    ret = recv(new_sd, (void*) &lmsg, sizeof (uint32_t), 0);
                    len = ntohl(lmsg); /* Rinconverto in formato host*/
                    ret = recv(new_sd, (void*) buffer, len, 0);
                    if (ret < 0) {
                        perror("Errore in fase di ricezione: \n");
                        continue;
                    }
                    /*ignoro il messaggio in quanto l'ip è bloccato
                    informo il client che il suo ip è bloccato*/
                    snprintf(buffer, BUFFER_SIZE, "Il tuo IP risulta ancora bannato, riprova tra : %f minuti", BAN_TIME - ciresult);
                    len = strlen(buffer)+1;
                    lmsg = htonl(len);
                    ret = send(new_sd, (void*) &lmsg, sizeof (uint32_t), 0);
                    /*invio risposta*/
                    ret = send(new_sd, (void*) buffer, len, 0);

                    if (ret < 0) {
                        perror("Errore in fase di invio: \n");
                        continue;
                    }
                    close(new_sd);
                }
            }

            pidc = fork();

            if (pidc == 0) {
                /* Processo figlio*/
                close(sd);
                memset(username, 0, BUFFER_SIZE);

                while (1) {
                    /*pulizia*/
                    memset(buffer, 0, BUFFER_SIZE);
                    lmsg = len = 0;
                    /* Attendo dimensione del comando*/
                    ret = recv(new_sd, (void*) &lmsg, sizeof (uint32_t), 0);
                    len = ntohl(lmsg); /* Rinconverto in formato host*/
                    /* Attendo il comando*/
                    ret = recv(new_sd, (void*) buffer, len, 0);
                    if (ret < 0) {
                        perror("Errore in fase di ricezione: \n");
                        continue;
                    }
                    strncpy(support, buffer, BUFFER_SIZE);
                    /*splitto il messaggio ricevuto*/
                    memset(split, 0, BUFFER_SIZE * BUFFER_SIZE);

                    slen = 0;
                    message_split(support, split, space, &slen);
                    memset(support, 0, BUFFER_SIZE);

                    /*analisi del comando arrivato*/
                    if (session_id[0] == '\0') {/*login non ancora effettuato*/
                        if (exactstrcmp(split[0], "!signup")) {

                            printf("Chimata a signup\n");
                            ret = signup(split[1], split[2]);
                            if (ret > 0)
                                strcpy(buffer, "Registrazione effettuata con successo");
                            else if (ret == 0)
                                strcpy(buffer, "Errore : username gia' registrato, prova a cambiare username");
                            else { /*errore durante la creazione del file registro_utenti.txt*/
                                perror("Errore durante signup ");
                                strcpy(buffer, "!esci");
                            }
                        } else if (exactstrcmp(split[0], "!login")) {
                            printf("Chimata a login\n");
                            ret = login(split[1], split[2], session_id, &attempts);
                            if (ret > 0) {/*login effettuato correttamente*/
                                strncpy(buffer, session_id, BUFFER_SIZE);
                                strncpy(betfile, split[1], BUFFER_SIZE);
                                strncpy(username, split[1], BUFFER_SIZE);
                                strcat(betfile, "/giocate_attive.txt");
                            } else if (ret == -1) /*errore nell'apertura del file -> possibile errore nessun uetente registrato*/
                                strcpy(buffer, "Errore: ti sei già registrato?\nSe non lo hai fatto effettualo con !signup");
                            else if (ret == -2) {/* username e password non trovati*/
                                strcpy(buffer, "Errore: username o password errati\nTentativi rimanenti : ");
                                snprintf(sattempts, 3, "%d", attempts);
                                strncat(buffer, sattempts, BUFFER_SIZE);
                            } else { /* ret == -3 -> tentativi terminati*/
                                fd = fopen("ip_bloccati.txt", "a");
                                if (fd == NULL) {
                                    perror("Errore nella login ");
                                    exit(-1);
                                }
                                /*ricopio l'ip che va bloccato*/
                                snprintf(buffer, BUFFER_SIZE, "%hd ", ntohs(cl_addr.sin_addr.s_addr));
                                /* ricopio l'orario di estrazione nel buffer di appoggio*/
                                snprintf(support, BUFFER_SIZE, "%ld \n", time(NULL));
                                /*concateno le due informazioni*/
                                strncat(buffer, support, BUFFER_SIZE);

                                /*ricopio nel file l'ip che è stato bloccato e il relativo timestamp*/
                                ret = fprintf(fd, "%s", buffer);

                                /*chiusura del file ip_bloccati*/
                                fclose(fd);

                                /*preparo messaggio di errore*/
                                strncpy(buffer, "Errore: numero di tentativi terminato, ritenta tra 30 minuti", BUFFER_SIZE);
                            }
                        } else if (exactstrcmp(split[0], "!vedi_estrazione")){
                            printf("Chimata a vedi_estrazione\n");
                            send_from_file = true;
                            /*calcolo la dimensione del file*/
                            file_len = file_size("estrazioni.txt", &lnumber);
                            /*aggiusto dimensione per tener conto della formattazione 
                             512 approssimazione della dim dell'estrazione * il numero di estrazioni da visualizzare*/ 
                            file_len = 512 * atoi(split[1]);
                            buffer_file = (char*) malloc(file_len);
                            if(buffer_file == NULL){
                            	perror("Errore in !vedi_estrazione");
                            	send_from_file = false;
                            	strcpy(buffer, "!esci");
                            }else{
				                if(atoi(split[1]) <= (lnumber/12)){
				                    vedi_estrazione(buffer_file, file_len, (lnumber/12) - atoi(split[1]), split[2]);
				                } else{
				                    vedi_estrazione(buffer_file, file_len, 0, split[2]);
				                    strncat(buffer_file,"\nIl numero di estrazioni selezionate non è presente",file_len);
				                }
		             		}
                        }else if(exactstrcmp(split[0], "!invia_giocata") ||
                                exactstrcmp(split[0], "!vedi_giocate") ||
                                exactstrcmp(split[0], "!vedi_vincite")) {
                            strncpy(buffer, "Prima di scommettere devi effettuare il login.", BUFFER_SIZE);
                        } else if (exactstrcmp(split[0], "!esci")) {
                            printf("Chimata a esci\n");
                            strcpy(buffer, "!esci");
                        } else {
                            printf("errore\n");
                            strcpy(buffer, "Comando inesistente, digita \"!help\" per una lista dei comandi accettati");
                        }
                    } else if (exactstrcmp(session_id, split[0])) {/*login effettuato*/
                        if (exactstrcmp(split[1], "!signup") || exactstrcmp(split[1], "!login")) {
                            strcpy(buffer, "Per utilizzare questi comandi esci dal tuo account");
                        } else if (exactstrcmp(split[1], "!invia_giocata")) {
                            printf("Chimata a invia_giocata\n");
                            if (betfile[0] == '\0')/*mancano i file utente*/
                                strcpy(buffer, "Prima di scommettere devi effettuare il login.");
                            else {
                                fd = fopen(betfile, "a");
                                /*inserisco nel file la scommessa */
                                if (fd == NULL) {
                                    perror("Errore in invia_giocata ");
                                    exit(-1);
                                }

                                i = 2;
                                snprintf(support, BUFFER_SIZE, "%ld", time(NULL));
                                while (i < slen - 1) {
                                    strcat(support, " ");
                                    strncat(support, split[i], BUFFER_SIZE);

                                    i++;
                                }
                                strcat(support, " ");
                                strncat(support, split[slen - 1], BUFFER_SIZE);
                                strcat(support, " \n");

                                if (fprintf(fd, "%s", support) < 0) {
                                    perror("Errore in invia giocata ");
                                    strcpy(buffer, "Giocata non accettata,riprovare");
                                } else {
                                    strcpy(buffer, "Giocata effettuata con successo");
                                }
                                fclose(fd);
                            }
                        } else if (exactstrcmp(split[1], "!vedi_giocate")) {
                            printf("Chimata a vedi_giocate\n");
                            send_from_file = true;
                            aggiorna_file_utente(username);
                            /*calcolo la dimensione del file*/
                            memset(support, 0, BUFFER_SIZE);
                            strncpy(support, username, BUFFER_SIZE);
                            strcat(support, (atoi(split[2]) == 0) ? "/giocate_effettuate.txt" : "/giocate_attive.txt");

                            file_len = file_size(support, &lnumber);
                            /*aggiusto dimensione per tener conto della formattazione */
                            file_len = file_len + ((5 + (4 + 8) * 5) * lnumber);
                            buffer_file = (char*) malloc(file_len);
                            if(buffer_file == NULL){
                            	perror("Errore in !vedi_giocate");
                            	send_from_file = false;
                            	strcpy(buffer, "!esci");
                            }else{
				                /*chiamo la vedi_giocate*/
				                if (vedi_giocate(support, buffer_file, file_len) < 0) {
				                    send_from_file = false;
				                    memset(buffer_file,0,file_len);
				                    free(buffer_file);
				                    strcpy(buffer, "Non sono presenti giocate di questo tipo");
				                }
		            		}
                        } else if (exactstrcmp(split[1], "!vedi_estrazione")) {
                            printf("Chimata a vedi_estrazione\n");
                            send_from_file = true;
                            /*calcolo la dimensione del file*/
                            file_len = file_size("estrazioni.txt", &lnumber);
                            /*aggiusto dimensione per tener conto della formattazione 
                             512 approssimazione della dim dell'estrazione * il numero di estrazioni da visualizzare */ 
                            file_len = 512 * atoi(split[2]);
                            buffer_file = (char*) malloc(file_len);
                            if(buffer_file == NULL){
                            	perror("Errore in !vedi_estrazione");
                            	send_from_file = false;
                            	strcpy(buffer, "!esci");
                            }else{
		                        if(atoi(split[2]) <= (lnumber/12)){
		                            vedi_estrazione(buffer_file, file_len, (lnumber/12) - atoi(split[2]), split[3]);
		                        } else{
		                            vedi_estrazione(buffer_file, file_len, 0, split[3]);
		                            strcat(buffer_file,"\nIl numero di estrazioni selezionate non è presente");
		                        }
		                    }
                        } else if (exactstrcmp(split[1], "!vedi_vincite")) {
                            printf("Chimata a vedi_vincite\n");
                            send_from_file = true;
                            aggiorna_file_utente(username);
                            memset(support, 0, BUFFER_SIZE);
                            strncpy(support, username, strlen(username)+1);
                            strcat(support, "/vincite.txt");

                            file_len = file_size(support, &lnumber);
                            /*aggiusto dimensione per tener conto della formattazione(128 approssimazione della dimesione del recap di vincita)*/
                            file_len = file_len + 256;
                            buffer_file = (char*) malloc(file_len);
                            if(buffer_file == NULL){
                            	perror("Errore in !vedi_vincite");
                            	send_from_file = false;
                            	strcpy(buffer, "!esci");
                            }else{
		                        /*chiamo la vedi_vincite*/
		                        if (vedi_vincite(support, buffer_file, file_len) < 0) {
		                            send_from_file = false;
		                            memset(buffer_file,0,file_len);
		                            free(buffer_file);
		                            strcpy(buffer, "Non sono presenti vincite, prova a scommettere o attendere la prossima estrazione");
		                        }
		                    }
                        } else if (exactstrcmp(split[1], "!esci")) {
                            printf("Chimata a esci\n");
                            strcpy(buffer, "!esci");
                        } else {
                            printf("errore\n");
                            strcpy(buffer, "Comando inesistente, digita \"!help\" per una lista dei comandi accettati");
                        }
                    } else {/*cliente non riconosciuto*/
                        strcpy(buffer, "!esci");
                    }
                    if (send_from_file) {
                        send_from_file = false;
                        /*invio dimensione*/
                        len = strlen(buffer_file) + 1;
                        lmsg = htonl(len);
                        ret = send(new_sd, (void*) &lmsg, sizeof (uint32_t), 0);
                        /*invio risposta*/
                        ret = send(new_sd, (void*) buffer_file, len, 0);

                        if (ret < 0) {
                            perror("Errore in fase di invio: \n");
                            continue;
                        }
                        memset(buffer_file,0,file_len);
                        free(buffer_file);
                    } else {
                        /*invio dimensione*/
                        len = strlen(buffer) + 1;
                        lmsg = htonl(len);
                        ret = send(new_sd, (void*) &lmsg, sizeof (uint32_t), 0);
                        /*invio risposta*/
                        ret = send(new_sd, (void*) buffer, len, 0);

                        if (ret < 0) {
                            perror("Errore in fase di invio: \n");
                            continue;
                        }

                        if (exactstrcmp(buffer, "!esci") || attempts == 0) {/*l'ip è stato bloccato, quindi va chiusa la connessione*/
                            if (attempts == 0) {
                                printf("Tentativi di login terminati, chiusura connessione con il client\n");
                            } else {
                                printf("Chiamata ad esci, chiusura connessione con il client\n");
                            }
                            break;
                        }
                    }
                }
                close(new_sd);
                exit(1);
            } else if (pidc > 0) {
                /* Processo padre*/
                close(new_sd);
            } else {
                printf("Errore nella creazione del processo che si occupa delle connessioni\n");
                exit(-1);
            }
        }
    } else {
        printf("Errore nella creazione del processo che si occupa lotto\n");
        exit(-1);
    }
}
