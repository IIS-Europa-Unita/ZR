bool itemChosen;
int fase, sottofase, ID, BoR;
float vai[3], zona[4], speed, stato[12], statoAvv[12], distAvvPrima, vel[3], dropError, R[3];

//FUNZIONE PER SETTARE UN VETTORE **FUNZIONA**
/*
-input: _float v[] = Vettore al quale assegniamo i valori di x, y e z
        _float x = Valore della coordinata x
        _float y = Valore della coordinata y
        _float z = Valore della coordinata z
-output: non ritorna direttamente valori, ma modifica il vettore passato con i valori di v[]
-descrizione:   _Assegno a v[] nelle sue diverse posizioni, le coordinate passate nei parametri seguendo questo ordine:
                v[0] = x / v[1] = y / v[2] = z
*/
void setV(float v[],float x,float y,float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

//FUNZIONE PER CALCOLARE UNA DISTANZA **FUNZIONA
/*
-input: _float a[] = Primo punto
        _float b[] = Secondo punto
-otuput: _Ritorna un valore float unico equivalente alla distanza dal punto "a" al punto "b"
-descrizione:   _Tramite una formula matematica, calcola la distanza fra il punto "a" e il punto "b"
*/
float dist(float a[],float b[])
{
	return sqrt(mathSquare(a[0] - b[0]) + mathSquare(a[1] - b[1]) + mathSquare(a[2] - b[2]));
}

//FUNZIONE PER RACCOLTA DATI **FUNZIONA**
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
	
	vel[0] = stato[3];
	vel[1] = stato[4];
	vel[2] = stato[5];

	speed = mathVecMagnitude(vel,3);
}

//FUNZIONE PER IL MOVIMENTO **FUNZIONA**
/*
-input:     non assume alcun valore in input
-output:    non ritorna alcun valore in output
-descrizione:   _All'inizio della funzione "muovi" viene inizializzato un vettore "vec" al quale viene assegnato la differenza tra la posizione
                del nostro satellite al punto al quale dobbiamo arrivare. Viene utilizzato nella funzione "setVelocityTarget"
                
                _Nell'"if" controlliamo se abbiamo rilasciato tutti gli SPS (e quindi la fase è diventata = 1):
                
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
    float out[3];
    float vec[3];
    mathVecSubtract(vec, vai, stato, 3);
    
    if (dist(vai, stato) > 0.85)
    {
        api.setVelocityTarget(vec);
        DEBUG (("GOTTA GO FAST"));
    }
    else
    {
        if (itemChosen)
        {
            if (dist(vai, stato) > R[game.getItemType(ID)])
                    api.setPositionTarget(vai);
                    
                else 
                    if (speed > 0.01) 
                        api.setPositionTarget(stato);
        }
        else
        {
            if (dist(vai, stato) > 0.2)
                api.setPositionTarget(vai);
            else
                api.setPositionTarget(stato);
        }
    }
    
    if (itemChosen || game.hasItem(ID))
        ruota();
}

//FUNZIONE PER LA ROTAZIONE **FUNZIONA**
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

//FUNZIONE PER L'ESECUZIONE DEL DOCKING **FUNZIONA**
/*
-input: non riceve alcun valore in input
-output: non ritorna alcun valore in output
-descrizione:   _All'interno del parametro dell'if, richiamiamo "checkDock()", che ritornerà un valore booleano:
                    _se checkDock() = true:
                                            _Eseguo il dock dell'item tramite la funzione "dockItem(ID)"
                    _se checkDock() = false:
                                            _Non eseguo alcuna operazione fino a quando checkDock non ritornerà "true"
*/
bool dock()
{   
    bool checkDoc;
    float distdock [6][2] = {	{.151,.173}, {.151,.173},
                               	{ .138 , .160 },{ .138 , .160 },
                          		{ .124 , .146 },{ .124 , .146 }
                          	};

    checkDoc = dist(stato, vai) > distdock[ID][0] && dist(stato, vai) < distdock[ID][1] && speed < .01? true:false; 

    if (checkDoc){
        game.dockItem(ID);
        if (game.hasItem(ID))
        {
            DEBUG(("Picking up? Don't mind if i do"));
            return true;    //Ho dockato
        }
        else 
            return false;   //Non ho dockato
    }
    else
        return false;
}

//FUNZIONE PER LA SCELTA DELL'OGGETTO
/*
-input:     non assume alcun valore in input
-output:    non ritorna alcun valore
-descrizione:   Comincio con vedere se ho tutti e 2 i gialli;se è cosi passo ai 
                MEDIUM,ecc.
                La funzione scelgoQuale determina quale dei 2 oggetti di un 
                determinato tipo conviene andare a prendere 
*/
void itemPriorityV2()
{
    int ogg = 0;
    float obj[3], obj2[3];
        
    do
    {
        //Abbiamo preso tutti e due e sono nella zona
        if (game.itemInZone(ogg) && game.itemInZone(ogg+1))
            ogg+=2;
        
        else
        {
            if (!game.itemInZone(ogg) && !game.itemInZone(ogg+1))
            {
            
                //Entrambi gli oggetti sono disponibili
                    if (dist(stato, obj)+dist(zona, obj) < dist(stato, obj2)+dist(zona, obj2))
                        ID = ogg;
                    else
                        ID = ogg+1;
                    
                    game.getItemLoc(obj, ID);
                    
                    if (dist(statoAvv, obj) < dist(stato, obj) || (game.hasItem(ID) == 2))
                        if (ID == ogg)
                            ID = ogg+1;
                        else
                            ID = ogg;
                    
                    itemChosen = true;
                    game.getItemLoc(vai, ID);
            }
            else
            {
                //Solo uno dei due disponibile    
                if (game.itemInZone(ogg))
                    ID = ogg+1;
                else 
                    ID = ogg;
                
                game.getItemLoc(obj, ID);
                
                if (dist(statoAvv, obj) > dist(stato, obj))
                {
                    game.getItemLoc(vai, ID);
                    itemChosen = true;
                }
                else
                    //Cambio colore
                    ogg+=2;
            }
        }
    }while (!itemChosen);
    
    DEBUG (("Order? I wouldn't mind the number %d, thanks", ID));
}

//FUNZIONE PER LO SGANCIO DI UN OGGETTO **FUNZIONA**
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
bool dropItem()
{
    float obj[3];
    setV(vai, zona[0], zona[1], zona[2]);
    game.getItemLoc(obj, ID);
    
    if (dist(vai, obj) <= dropError)
    {
        game.dropItem();
        itemChosen = false;
        DEBUG(("There you go... STAY THERE OK? Don't move."));
        return true;
    }
    else
    	return false;
}

//FUNZIONE PER L'INIZIALIZZAZIONE DELLE VARIABILI **FUNZIONA**
/*
-input: non assume alcun valore in input
-output: non ritorna alcun valore in output
-descrizione: _Viene utilizzata per l'inizializzazione delle variabili globali
*/
void init()
{
    //Azzero vai
    vai[0] = 0;
    vai[1] = 0;
    vai[2] = 0;
    
    //Valori dei raggi massimi per oggetti LARGE (0); MEDIUM (1); SMALL (2)
    R[0] = .27;
    R[1] = .25;
    R[2] = .25;
    
    //Prelevo il mio stato
	api.getMyZRState(stato);
	
	//Capisco quale sfera siamo
	BoR = stato[1] > 0 ? 1 : 0;     //1 = Blu / 0 = Rossa
	
	//Inizializzo le variabili
	fase = sottofase = ID = dropError = 0;
	itemChosen = false;
	
	DEBUG(("Should this be something easy? I hope so"));
}

//Funzione per l'applicazione della strategia e delle funzioni **FUNZIONA**
void loop()
{
    //Prelevo i dati all'inizio dello switch
    getDati();
    //Lo siwtch controlla il drop degli sps
    switch(fase){
        //case 0: Droppo gli sps alla posizione stabilita tramite setV()
        case 0:
                switch (sottofase)
                {
                    case 0: //Droppo il primo SPS
                            game.dropSPS();
                            sottofase++;
                            if(BoR) setV(vai,-0.6,0.5,0.3);
                                else setV(vai,0.6,-0.5,0.3);
                            //setV(vai, -0.25, 0.45, .0); **Alternativo
                            break;
                            
                    case 1: //Droppo il secondo SPS se la distanza dalla destinazione è minore di 0.2
                            if (dist(vai, stato) < 0.2)
                            {
                                game.dropSPS();
                                sottofase++;
                                //setV(vai, -0.25, -0.1, -0.25); **Alternativo
                            }
                            break;
                            
                    case 2: //Droppo il terzo SPS se la distanza dalla destinazione è minore di 0.2
                            if (dist(vai, stato) < 0.2)
                            {
                                     //Riazzero la sottofase -> Viene utilizzato in fase2
                                sottofase=0;
                                fase++;             //Cambio fase
                            }
                            break;
                }
                break;
       
        case 1:
                switch(sottofase)
                {
    	            //Vado a prendere un oggetto
             	   	case 0:
        	                //Se non ho scelto un oggetto e lo devo ancora scegliere, eseguo l'itemPriority
            	        	if (!itemChosen)
                	           	itemPriorityV2();
                       
                    	    //l'oggetto verso cui andavamo è stato preso dall'avversario
                    	    if (game.hasItem(ID) == 2)
                        			itemChosen = false;
                            //Tento di fare il docking;se ci riesco vado a posarlo in zona di assembleaggio
                        	else 
                        	    if (!game.hasItem(ID))
                            	    if (dock())
                            	    {
                            	        if (game.getNumSPSHeld() > 0)
                            	        {
                                	        game.dropSPS();
    		                                game.getZone(zona);
    		                                
    		                                dropError = 0.037;
                            	        }
                            	        sottofase++;
                            	    }
               			    break;
        			
        			//Vado a posare l'oggetto	
        			case 1: //Se sono troppo vicino alla zona, mi allontano
                            if ((dist(zona, stato) < dropError) && (game.hasItem(ID) == 1))
                            {
                                setV (out, 0.0, 0.0, 0.0);
                                api.setPositionTarget(out);
                            }
        			        //Porto sottofase a a 0 quando l'ho posato, per ricomincaire con un altro oggetto
        				    if (dropItem())
        					    sottofase = 0;
        			        break;
    			}
    			break;
    }
    
    if (((dist(zona, statoAvv) < 0.5) && (dist(zona, stato) < dist(zona, statoAvv))) && !game.hasItem(ID))
    {
        if ((distAvvPrima > dist(zona, statoAvv)))
            api.setPositionTarget(zona);
    }
    else
        if (((game.getFuelRemaining() < 7) || (game.itemInZone(0) && game.itemInZone(1))) && (!game.hasItem(ID) || (game.hasItem(ID) == 2)))
            api.setPositionTarget(zona);
        else   
            muovi();    //Si muove verso vai[]
    
    if (fase == 1)
        distAvvPrima = dist(zona, statoAvv);
}
