/*
Da cambiare: LARGE, MEDIUM e SMALL come vettore booleano;
Se l'avversario prende oggetto che dovevamo prendere noi, aspettiamo che lo rilasci e lo rubiamo;
Controllare chooseItem e itemChosen se servono o meno;
Cercare di non prendere oggetto che abbiamo già posato;
*/
bool LARGE, MEDIUM, SMALL, spsAllDrop, itemChosen, chooseItem;
int fase, sottofase, ID, BoR;
float vai[3], zona[4], speed, stato[12], statoAvv[12], facing[3], posAvv[3], vel[3], oggetto[3], R[3];

//Funzione per prelevare dati
void getDati()
{
    //Aggiorno dati miei
    api.getMyZRState(stato);
    //Aggiorno dati avversario
	api.getOtherZRState(statoAvv);
	
	//Aggiorno i dati e ottengo mia posizione e pos avversario + direzione
	for(int i = 0; i < 3; i++)
	{
		facing[i] = stato[i + 6];
		posAvv[i] = statoAvv[i];
		vel[i] = stato[i + 3];
	}
	
	//Aggiorno la velocità
	speed = mathVecMagnitude(vel,3);
	
	//Aggiorno quali oggetti sono disponibili
	if(game.hasItemBeenPickedUp(ID) || game.hasItem(ID)==2)
        switch (ID)
        {
            case 0: case 1: LARGE = FALSE; break;
            case 2: case 3: MEDIUM = FALSE; break;
            case 4: case 5: SMALL = FALSE;
        }
}
//Funzione per settare un vettore
void setV(float v[],float x,float y,float z)/*Definitivo*/
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}
//Funzione per il calcolo della distanza
float dist(float a[],float b[])/*Definitivo*/
{
	return sqrt(mathSquare(a[0] - b[0]) + mathSquare(a[1] - b[1]) + mathSquare(a[2] - b[2]));
}
//Funzione per il movimento
void muovi()
{
    float vec[3];
		mathVecSubtract(vec,vai,stato,3);
	
	switch (spsAllDrop){
	                
	    case true:  //Se non abbiamo l'oggetto
	                if (game.hasItem(ID) == 0)
	                {
	                    //Prendo posizione oggetto con ID stabilito da itemPriority
	                    game.getItemLoc(vai, ID);
    	                  
    	                //Se la distanza è maggiore del raggio maggiore per prendere l'oggetto specifico
    	                if (dist(vai, stato) > R[game.getItemType(ID)])
    	                    api.setPositionTarget(vai); //Viaggio verso l'oggetto
    	                //Altrimenti se è minore e la velocità è maggiore di 0.01
    	                else if (speed > 0.01) 
    	                        api.setPositionTarget(stato);   //Rallento
    	                        
    	                /*if ((dist(vai, stato) < r[game.getItemType(ID)]) && speed > 0.01)
    	                    api.setPositionTarget(stato);
    	                else if((dist(vai, stato) > R[game.getItemType(ID)])) 
    	                        api.setPositionTarget(vai);
    	                     else if (speed < 0.009) 
    	                            api.setPositionTarget(vai);
    	                            else api.setPositionTarget(stato);*/
	                }
	                //Altrimenti se abbiamo l'oggetto
    	            else if (game.hasItem(ID)==1)
    	            {
    	                //Richiamo la funzione dropItem per assegnare la posizione della zona e controllare quando rilasciare l'oggetto
    	                dropItem();
    	  
    	                //Se la distanza è minore di 0.15, attiva il "freno" per avvicinarsi alla zona
    	                if (dist(vai, stato) < 0.13)
    	                    api.setPositionTarget(stato);
    	                //Altrimenti utilizza il setPosition per avvicinarsi alla zona
    	                else api.setPositionTarget(vai);
    	            }
    	                //Altrimenti se l'avversario ha l'oggetto -> Prendo un nuovo oggetto (da controllare strategicamente)
    	                else 
    	                {
    	                   itemChosen=false;
    	                   chooseItem=true;
    	                }
    	            
    	            //Ruota viene sempre richiamato qui poichè è fondamentale
    	            ruota();
	                break;
	                
	                //Se la distanza alla posizione è minore di 0.13, rallenta per fermarsi
	    case false: if (dist(vai, stato) < 0.18)
	                    api.setPositionTarget(stato);
	                //Altrimenti, se la posizione è maggiore di 0.50, aumenta la velocità
	                else if (dist(vai, stato) > 0.65)
	                     {
	                         api.setVelocityTarget(vec);
	                         api.setPositionTarget(vai);
	                     }
	                     //Altrimenti, se si trova tra 0.13 e 0.50, utilizza il setPosition per rallentare la velocità senza fermarsi
	                     else api.setPositionTarget(vai);
	}
}
//Funzione per la rotazione
void ruota()
{
    float vett[3];
    
    mathVecSubtract(vett,vai,stato,3);
    mathVecNormalize(vett,3);
    api.setAttitudeTarget(vett);
}
//Funzione per il docking
void dock()
{
    //Switch per eseguire il docking in base alla possibilità determinata da checkDock()    
    switch (checkDock()){
        
        case true:  //Esegue il docking dell'oggetto
                    game.dockItem();
                    break;
                    
        case false: DEBUG (("Dock impossibile")); //Parte da aggiungere
    }
}
//Funzione per il controllo della possibilità di docking
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
//Funzione per la scelta dell'item da prendere in base a condizioni di distanza
void itemPriority()
{
    
    float d[3];
    float d1[3];
    
    //0-1 sono i large, 2-3 i medium e 4-5 gli small
    if(!LARGE)
      ID=2;     //Se il Large è stato preso, setto ID a 2 per prendere i Medium
    if(!MEDIUM)
      ID=4;     //Se il Medium è stato preso, setto ID a 4 per prendere i Small
    if(!SMALL){
      MEDIUM = true;
      ID=2; //se ho preso anche uno Small, passo di nuovo al Medium e poi all'altro Small
    }
    
    //Prelevo la posizione degli oggetti in base all'ID selezionato in precedenza
    game.getItemLoc(d,ID);
    //Tengo conto che gli oggetti hanno ID successivi
    game.getItemLoc(d1,(ID+1));
    
    if ((dist(stato, d)+dist(d, zona)) > (dist(stato, d1)+dist(d, zona)))
        ID++;
    //Controllo dell'oggetto più conveniente da prendere
    /*if((dist(stato, d)+dist(d, zona)) > (dist(stato, d1)+dist(d1, zona)) && !game.hasItemBeenPickedUp(ID+1))
        ID++;   //Se la funzione è vera, ovvero non posso prendere ID, prendo ID+1*/
        
    //L'oggetto da prendere è stato scelto e non devo più scegliere un oggetto
    itemChosen = true;
    chooseItem = false;
}
//Funzione per rilasciare l'oggetto
void dropItem()
{
    //Settiamo al vettore vai la zona per il rilascio dell'oggetto
    setV(vai, zona[0], zona[1], zona[2]);
    
    if (dist(vai,stato) < 0.15)
    {
        //Rilascia l'oggeto nella zona
        game.dropItem();
            
        //Avendo lasciato l'oggetto precedentemente preso, devo sceglierne un altro
        chooseItem = true;
        itemChosen = false;
    }
}
//Funzione per l'inizializzazione delle variabili (eseguito solo una volta)
void init()
{
    //Azzero vai
    for (int i = 0; i < 3; i++)
        vai[i]=0;
    
    //Valori dei raggi massimi per oggetti LARGE (0); MEDIUM (1); SMALL (2)
    R[0] = .22;
    R[1] = .22;
    R[2] = .23;
    
    //Prelevo il mio stato
	api.getMyZRState(stato);
	
	//Capisco quale sfera siamo
	BoR = stato[1] > 0 ? 'R' : 'B'; 
	
	//Inizializzo le variabili
	fase = sottofase = ID = 0;
	spsAllDrop = itemChosen = false;
	LARGE = MEDIUM = SMALL = chooseItem = true;
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
                setV(vai,-0.3,-0.5,0);
                //setV(vai, -0.25, 0.45, .0); **Alternativo
            }
            //Droppo il secondo SPS
            else if (sottofase == 1 && dist(vai,stato) <0.1)
            {
                game.dropSPS();
                sottofase++;
                setV(vai,-0.3,0.5,0);
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
                if (itemChosen == false && chooseItem == true)
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
