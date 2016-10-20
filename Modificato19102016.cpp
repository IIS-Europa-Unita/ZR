bool spsAllDrop, itemChosen, TYPE[3];
int fase, sottofase, ID, BoR;
float vai[3], zona[4], speed, stato[12], statoAvv[12], facing[3], posAvv[3], vel[3], oggetto[3], R[3];

//FUNZIONE PER RACCOLTA DATI
/*
-input: non riceve alcun dato esterno
-output: non ritorna valori
-funzionalità:  _Aggiorna i propri dati e quelli dell'avversario, riguardanti la posizione, la velocità e dove sto guardando.
                _Tiene sotto controllo quali sono gli oggetti che abbiamo e quelli che non abbiamo, in base alla loro presenza nella zona
                 di scarico o se li stiamo trasportando.
                 Nel caso che non siano presenti nella zona o non li abbiamo con noi, li segna non presenti
*/
void getDati()
{
    api.getMyZRState(stato);
	api.getOtherZRState(statoAvv);
	
	for(int i = 0; i < 3; i++)
	{
		facing[i] = stato[i + 6];
		posAvv[i] = statoAvv[i];
		vel[i] = stato[i + 3];
	}
	
	speed = mathVecMagnitude(vel,3);
	
	if(game.hasItem(ID) || game.itemInZone(ID))
        switch (ID)
        {
            case 0: case 1: TYPE[0] = true; break;
            case 2: case 3: TYPE[1] = true; break;
            case 4: case 5: TYPE[2] = true;
        }
    else
        switch (ID)
        {
            case 0: case 1: TYPE[0] = false; break;
            case 2: case 3: TYPE[1] = false; break;
            case 4: case 5: TYPE[2] = false;
        }
    
    DEBUG(("LARGE: %d / MEDIUM: %d / SMALL: %d", TYPE[0], TYPE[1], TYPE[2]));
}
//FUNZIONE PER SETTARE UN VETTORE
/*
-input: _float v[] = Vettore al quale assegniamo i valori di x, y e z
        _float x = Valore della coordinata x
        _float y = Valore della coordinata y
        _float z = Valore della coordinata z
-output: non ritorna direttamente valori, ma modifica il vettore passato con i valori di v[]
-descrizione:   _Assegno a v[] nelle sue diverse posizioni, le coordinate passate nei parametri seguendo questo ordine:
                v[0] = x / v[1] = y / v[2] = z
*/
void setV(float v[],float x,float y,float z)/*Definitivo*/
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}
//FUNZIONE PER CALCOLARE UNA DISTANZA
/*
-input: _float a[] = Primo punto
        _float b[] = Secondo punto
-otuput: _Ritorna un valore float unico equivalente alla distanza dal punto "a" al punto "b"
-descrizione:   _Tramite una formula matematica, calcola la distanza fra il punto "a" e il punto "b"
*/
float dist(float a[],float b[])/*Definitivo*/
{
	return sqrt(mathSquare(a[0] - b[0]) + mathSquare(a[1] - b[1]) + mathSquare(a[2] - b[2]));
}
//FUNZIONE PER IL MOVIMENTO
/*
-input:     non assume alcun valore in input
-output:    non ritorna alcun valore in output
-descrizione:   _All'inizio della funzione "muovi" viene inizializzato un vettore "vec" al quale viene assegnato la differenza tra la posizione
                del nostro satellite al punto al quale dobbiamo arrivare. Viene utilizzato nella funzione "setVelocityTarget"
                
                _Nell'"if" controlliamo se abbiamo rilasciato tutti gli SPS:
                
                    _se Sì: 
                            _Nel caso che ci manchi l'oggetto, ci muoviamo verso la posizione definita dall'ID 
                                (trovato tramite la funzione itemPriority, richiamata nel loop).
                                Durante il movimento, teniamo conto della nostra velocità e la nostra distanza rispetto all'oggetto:
                                
                                    _se la distanza è maggiore del raggio massimo: _Ci muoviamo verso la posizione dell'oggetto
                                    _altrimenti se la velocità è maggiore di 0.01 m/s, rallentiamo
                                    
                                Questo ci aiuta a prendere l'oggetto molto velocemente.
                                
                            _Nel caso che avessimo l'oggetto, richiamiamo la funzione "dropItem", la quale individuerà la posizione della zona
                                attribuendola al vettore "vai" e controllerà la funzione di drop dell'oggetto, che avverrà quando le condizioni 
                                interne di "dropItem" saranno soddisfatte.
                                Dopodichè:
                                
                                    _se la distanza dalla zona di scarico è minore di 0.13m, il satellite rallenta o rimane fermo
                                    _altrimenti, viaggia verso la posizione della zona di drop
                                    
                            _Nel caso in cui l'avversario ha l'oggetto che noi dovremmo prendere, ri-scegliamo quale oggetto prendere
                            
                            _Alla fine, richiamiamo la funzione "ruota()", che in questo caso va eseguito sempre, poichè abbiamo necessità
                            di avere il satellite rivolto costantemente verso l'oggetto o la zona di scarico
                            
                    _se No: 
                            _Dopo aver assegnato nel "loop" il punto in cui dirigermi per rilasciare il primo/secondo/terzo SPS
                            controllo la distanza del nostro satellite da tale punto:
                            
                                _se la distanza è minore di 0.13m, il satellite rallenta
                                _se la distanza è maggiore di 0.60m, il satellite utilizzerà la funzione "setVelocityTarget"
                                per muoversi velocemente verso il punto
                                _se la distanza è compresa fra 0.13m e 0.60m, il satellite utilizza "setPositionTarget" per rallentare
                                prima di arrivare al punto e evitare quindi di mancarlo
                            
*/
void muovi()
{
    float vec[3];
		mathVecSubtract(vec,vai,stato,3);
	
	if (spsAllDrop)
	{
        if (game.hasItem(ID) == 0)
        {
            game.getItemLoc(vai, ID);
              
            if (dist(vai, stato) > R[game.getItemType(ID)])
                api.setPositionTarget(vai);
                
            else if (speed > 0.01) 
                    api.setPositionTarget(stato);
        }
        else if (game.hasItem(ID)==1)
        {
            dropItem();

            if (dist(vai, stato) < 0.24)
                api.setPositionTarget(stato);
            else api.setPositionTarget(vai);
        }
            else 
            {
               itemChosen=false;
            }
        ruota();
	}
    else 
        if (dist(vai, stato) < 0.13)
            api.setPositionTarget(stato);
        else 
            if (dist(vai, stato) > 0.60)
            {
                api.setVelocityTarget(vec);
                api.setPositionTarget(vai);
            }
            else 
                api.setPositionTarget(vai);
	
	        
}
//FUNZIONE PER LA ROTAZIONE
/*
-input: non riceve alcun valore in input
-output: non ritorna alcun valore in output
-descrizione:   _Viene inizializzato un vettore float "vett"
                _Viene eseguito il calcolo matematico con le varie assegnazioni per trovare il vettore normalizzato
                _Viene richiamata la funzione "setAttitudeTarget" immettendo come parametro il vettore normalizzato
                così facendo il satellite ruota fino a guardare il punto calcolato
*/
void ruota()
{
    float vett[3];
    
    mathVecSubtract(vett,vai,stato,3);
    mathVecNormalize(vett,3);
    api.setAttitudeTarget(vett);
}
//FUNZIONE PER L'ESECUZIONE DEL DOCKING
/*
-input: non riceve alcun valore in input
-output: non ritorna alcun valore in output
-descrizione:   _All'interno del parametro dell'if, richiamiamo "checkDock()", che ritornerà un valore booleano:
                    _se checkDock() = true:
                                            _Eseguo il dock dell'item tramite la funzione "dockItem(ID)"
                    _se checkDock() = false:
                                            _Non eseguo alcuna operazione fino a quando checkDock non ritornerà "true"
*/
void dock()
{
    if (checkDock())
        game.dockItem(ID);
}
//FUNZIONE PER IL CONTROLLO DELLA POSSIBILITA' DI DOCKING
/*
-input: non riceve alcun valore in input
-output:    _true = è possibile eseguire il docking
            _false = non è possibile eseguire il docking
-descrizione:   _Uno switch permette di controllare, in base all'ID scelto tramite l'"itemPriority", quali sono i range e la velocità
                a cui è consentito il docking (in base al tipo di oggetto):
                
                _Se questi range vengono rispettati, la funzione ritorna "true"
                _Se questi range non vengono rispettati, ritorna "false"
*/
bool checkDock()
{
    switch (ID){
        case 0: case 1: if (dist(stato, vai)>.151 && dist(stato, vai)<.173 && speed < .01)
                            return true;
                        else return false;
                        break;
                        
        case 2: case 3: if (dist(stato, vai)>.138 && dist(stato, vai)<.160 && speed < .01)
                            return true;
                        else return false;
                        break;
                        
        case 4: case 5: if (dist(stato, vai)>.124 && dist(stato, vai)<.146 && speed < .01)
                            return true;
                        else return false;
    }
}
//FUNZIONE PER LA SCELTA DELL'OGGETTO
/*
-input:     non assume alcun valore in input
-output:    non ritorna alcun valore
-descrizione:   _Vengono inizializzati all'inizio due vettori, "obj1" e "obj2", che conterrano le coordinate di due oggetti dello stesso tipo
                _Gli if annidati controllano quali oggetti sono stati presi e quali no:
                
                    _se TYPE[0] (oggetto LARGE) = false (ovvero non è stato preso): 
                        _Attribuisco il valore di 0 a ID, che è il primo dei due oggetti LARGE
                        
                    _altrimenti se TYPE[1] (oggetto MEDIUM) = false (ovvero non è stato preso):
                        _Attribuisco il valore di 2 a ID, che è il primo dei due oggetti MEDIUM
                        
                    _altrimenti se TYPE[2] (oggetto SMALL) = false (ovvero non è stato preso):
                        _Attribuisco il valore di 4 a ID, che il primo dei due oggetti SMALL
                
                _Dopo che è stato scelto il tipo di oggetto, vengono attribuiti a "obj1" e "obj2" le posizioni corrispettive di ID e ID+1
                _Se la distanza dall'oggetto "obj1"(ID) è maggiore rispetto alla distanza dall'oggetto "obj2"(ID+1) o l'oggetto "obj1"(ID) è stato preso:
                    _Cambio il valore di ID in ID+1
*/
void itemPriority()
{
    float obj1[3], obj2[3];
    
    if (!TYPE[0])   //LARGE
        ID = 0;
    else 
        if (!TYPE[1])   //MEDIUM
            ID = 2;
        else
            if (!TYPE[2])   //SMALL
            {
                ID = 4;
                TYPE[0] = false;
            }
    
    game.getItemLoc(obj1, ID);
    game.getItemLoc(obj2, ID+1);
    
    if ((dist(obj1, stato) + dist(obj1, zona)) > (dist(obj2, stato) + dist(obj2, zona)) || game.itemInZone(ID))
            ID++;
    
    itemChosen = true;
}
//FUNZIONE PER LO SGANCIO DI UN OGGETTO
/*
-input: non assume alcun valore in input
-output: non ritorna alcun valore in output
-descrizione:   _Si inizializza un vettore "obj", al quale verrà attribuito costantemente la posizione dell'oggetto "ID"
                _Settiamo il vettore "vai" tramite la funzione "setV" con le coordinate della zona di scarico
                _Se la distanza tra la zona di scarico e l'oggetto è minore di 0.045m:
                
                    _Viene rilasciato l'oggetto e itemChosen torna a "falso", poichè l'oggetto scelto ora è stato scaricato
                    e di conseguenza dobbiamo sceglierne uno nuovo
                    
                _Altrimenti non succede nulla
*/
void dropItem()
{
    float obj[3];
    setV(vai, zona[0], zona[1], zona[2]);
    game.getItemLoc(obj, ID);
    
    if (dist(vai, obj) < 0.035)
    {
        game.dropItem();
        
        itemChosen = false;
    }
}
//FUNZIONE PER L'INIZIALIZZAZIONE DELLE VARIABILI
/*
-input: non assume alcun valore in input
-output: non ritorna alcun valore in output
-descrizione: _Viene utilizzata per l'inizializzazione delle variabili globali
*/
void init()
{
    //Azzero vai
    for (int i = 0; i < 3; i++)
        vai[i]=0;
    
    //Valori dei raggi massimi per oggetti LARGE (0); MEDIUM (1); SMALL (2)
    R[0] = .23;
    R[1] = .23;
    R[2] = .23;
    
    //Prelevo il mio stato
	api.getMyZRState(stato);
	
	//Capisco quale sfera siamo
	BoR = stato[1] > 0 ? 1 : 0;     //1 = Blu / 0 = Rossa
	
	//Inizializzo le variabili
	fase = sottofase = ID = 0;
	spsAllDrop = itemChosen = false;
	
	//Tipo di oggetto, 0 = LARGE / 1 = MEDIUM / 2 = SMALL; true = tenuto da noi / false = non tenuto da noi
	TYPE[0] = false;
    TYPE[1] = false;
    TYPE[2] = false;
}
//Funzione per l'applicazione della strategia e delle funzioni
void loop()
{
    //Prelevo i dati all'inizio dello switch
    getDati();
    //Lo siwtch controlla il drop degli sps
    switch(fase){
        //case 0: Droppo gli sps alla posizione stabilita tramite setV()
        case 0:
            //Droppo il primo SPS
            if (sottofase == 0)
            {
                game.dropSPS();
                sottofase++;
                if(BoR)
                    setV(vai,-0.3,-0.5,0);
                else setV(vai,0.3,0.5,0);
                //setV(vai, -0.25, 0.45, .0); **Alternativo
            }
            //Droppo il secondo SPS
            else if (sottofase == 1 && dist(vai,stato) <0.1)
            {
                game.dropSPS();
                sottofase++;
                if(BoR)
                    setV(vai,-0.3,0.5,0);
                else setV(vai,0.3,-0.5,0);
                //setV(vai, -0.25, -0.1, -0.25); **Alternativo
            }
            //Droppo il terzo SPS
            else if(sottofase == 2 && dist(vai,stato) < 0.1)
            {
                game.dropSPS();
                spsAllDrop = true;  //Tutti gli SPS sono stati droppati
                sottofase = 0;      //Riazzero la sottofase -> Viene utilizzato in fase2
                fase++;             //Cambio fase
            }
       
        case 1:
            //Trovo la zona
            if (sottofase == 0)
            {
                game.getZone(zona);
                sottofase++;
            }
            //Prendo e posiziono gli oggetti
            else if (sottofase == 1)
            {
                //Se non ho scelto un oggetto e lo devo ancora scegliere, eseguo l'itemPriority
                if (!itemChosen)
                   itemPriority();
                DEBUG(("ID: %d", ID));
               
                if (game.hasItem(ID) == 0)
                    {
                        dock();     //funzione per il docking
                    }
            }
            break;
       
        default:    DEBUG(("ERROR"));
    }
    
    DEBUG (("/x:%f y:%f z:%f \n/ vel: %f \n/ fase: %d", vai[0], vai[1], vai[2], speed, fase));
    muovi();    //Si muove verso vai[]
}
  
