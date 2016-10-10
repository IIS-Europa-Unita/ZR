char BoR; // R se sfera rossa, B se blu
int fase,sottofase;
float stato[12];
float statoAvv[12];
float vai[3];
float punta[3];
float posAvv[3];
float pos[3];
float facing[3];
float vel[3];
float speed;
float zona[4];
bool LARGE, MEDIUM, SMALL;
int ID;
bool spsAllDrop;

void allontana(){
   for(int i=0; i<3; i++)
     if(vai[i]>0)
       vai[i] = vai[i]-0.03;
     else{
       vai[i] = abs(vai[i])-0.03;
       vai[i] = -vai[i];
     }
}

void avvicina(){
    for(int i=0; i<3; i++)
     if(vai[i]>0)
       vai[i] = vai[i]+0.03;
     else{
       vai[i] = abs(vai[i])+0.03;
       vai[i] = -vai[i];
     }
}

float dist(float a[],float b[])/*Definitivo*/{
	return sqrt(mathSquare(a[0] - b[0]) + mathSquare(a[1] - b[1]) + mathSquare(a[2] - b[2]));
}

void setV(float v[],float c[])/*Definitivo*/{
	v[0] = c[0];
	v[1] = c[1];
	v[2] = c[2];
}
void setV(float v[],float x,float y,float z)/*Definitivo*/{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

bool oOB(float ptc[])/*Definitivo*/{
	if((fabsf(ptc[0]) > 0.75) || (fabsf(ptc[1]) > 0.75) || (fabsf(ptc[2]) > 0.75))
      return true;
    return false;
}

void ruota(){
	float v[3];
	if(spsAllDrop && dist(pos,vai) < 0.5){//per risparmiare fuel
	  mathVecSubtract(v,punta,pos,3);
	  mathVecNormalize(v,3);
	  api.setAttitudeTarget(v);
	}
}

void muovi(){
	if(oOB(pos)){
		for(int i = 0; i < 3; i++)
			vai[i] = 0;
	}
	else{
		float vec[3],d = dist(vai,pos);
		DEBUG(("%f",d));
		mathVecSubtract(vec,vai,pos,3);
		switch(spsAllDrop)
		{
		    case true: 
		      if(d > 0.267)
		        api.setPositionTarget(vai);
		      else 
		        api.setPositionTarget(pos);
		    break;
		    case false:	
		      if(d > 0.50)
			    api.setVelocityTarget(vec);
		      api.setPositionTarget(vai);
		      if(d < 0.13)
			    api.setPositionTarget(pos); //sostituisce frena())
			break;
		}
	}
}

int checkDock(int id)/*verifico condizioni per docking*/{
	float targ[3];
	game.getItemLoc(targ,id); //Posizione dell'oggetto
	float targD = dist(targ,pos); //Distanza dall'oggetto 
	if(((id == 0 || id == 1) && targD < 0.151) ||
	((id == 2 || id == 3) && targD < 0.138) ||
	((id == 4 || id == 5) && targD < 0.124)){
		DEBUG(("Do: vicini"));
		return 1;
	}
	if(((id == 0 || id == 1) && targD > 0.173) ||
	((id == 2 || id == 3) && targD > 0.160) ||
	((id == 4 || id == 5) && targD > 0.146)){
		DEBUG(("Do: lontani"));
		return 2;
	}
	if(speed > 0.01){
		DEBUG(("Do: Velocita"));
		return 3;
	}
	return 0;
}

void dock(int id){
    switch (checkDock(id)){
        case 0:
          game.dockItem();
        break;
        case 1:
          allontana();
        break;
        case 2:
          if(dist(pos,punta) < 0.4)
            avvicina();
          else
            api.setPositionTarget(vai);
        break;
        case 3:
          api.setPositionTarget(pos);
        break;
        default: DEBUG (("ERRORE")); break;
    }
}

void init(){
	for(int i = 0; i < 3; i++)vai[i] = punta[i] = 0;
	api.getMyZRState(stato);
	BoR = stato[1] > 0 ? 'R' : 'B'; //capisco quale sfera siamo
	fase = sottofase = 0;
	spsAllDrop = false;
	LARGE = true;
	MEDIUM = true;
	SMALL = true;
}

void inizio(){
	api.getMyZRState(stato); //aggiorno dati miei
	api.getOtherZRState(statoAvv); //aggiorno dati avversario
	for(int i = 0; i < 3; i++){
		pos[i] = stato[i];
		facing[i] = stato[i + 6];
		posAvv[i] = statoAvv[i];
		vel[i] = stato[i + 3];
	}//ottengo pos + dir miei e pos avversario
	speed = mathVecMagnitude(vel,3);
}

void fine(){
	ruota(); //rotazione verso punta[]
	muovi(); //spostamento verso vai[]
}

int itemPriority(){
  /*float large[3]={0.375, 0.727, 0.2}; //massa, accelerazione, punteggio
    float medium[3]={0.25, 0.8, 0.15};
    float small[3]={0.125, 0.889, 0.1};*/
    float d[3]={0.0};
    float d1[3]={0.0};
    int i=0;
    
    if(LARGE == false)
      i+=2;              //0-1 sono i large, 2-3 i medium e 4-5 gli small, per quello incremento i così
    if(MEDIUM == false)
      i+=2;
    if(SMALL == false){
      MEDIUM = true;     //se ho preso anche uno small, passo di nuovo al medium e poi all'altro small
      i=2;
    }
    
    if(i==0 && (game.hasItemBeenPickedUp(i) || game.hasItemBeenPickedUp(i+1)))
      LARGE = false;
    if(i==2 && (game.hasItemBeenPickedUp(i) || game.hasItemBeenPickedUp(i+1)))
      MEDIUM = false;
    if(i==4 && (game.hasItemBeenPickedUp(i) || game.hasItemBeenPickedUp(i+1)))
      SMALL = false;
      
    game.getItemLoc(d,i);
    game.getItemLoc(d1,(i+1));
    if((dist(pos, d)+dist(d, zona)) > (dist(pos, d1)+dist(d1, zona)) && game.hasItemBeenPickedUp(i+1)){    //restituisce l'id dell'item più conveniente da andare a prendere
       return (i+1);
    }else
       return i;
}

void loop(){
    inizio();
	switch(fase){
        case 0://drop SPSs, i cosi per sapere dove si trova la zhohona
            if (sottofase==0){
                game.dropSPS();
                sottofase++;
                setV(vai, -0.3, -0.5, 0);
                //game.getItemLoc(vai,1);
            }
            else if (sottofase==1 && dist(vai,pos)<0.1){
                game.dropSPS();
                sottofase++;
                setV(vai, -0.3, 0.3, 0);
                //game.getItemLoc(vai,5);
            }
            else if(sottofase==2 && dist(vai,pos)<0.1){
                game.dropSPS();
                spsAllDrop = true;
                game.getZone(zona);
                sottofase=0;
                fase++;
            }
        break;
        case 1://getZone
            if(!game.getZone(zona)){//se true, carica anche la posizione della sfera in zona
                fase--;
            }
        break;
        default:
        DEBUG(("ERROR"));
	}
	if(spsAllDrop == true && game.hasItem(ID) == 0){
          ID = itemPriority();
          DEBUG (("ID: %d", ID));
          game.getItemLoc(vai, ID);
          game.getItemLoc(punta, ID);
          dock(ID);
    }
    else if(spsAllDrop){
        game.getZone(zona);
        setV(vai, zona);
        setV(punta, vai);
        if(dist(pos,vai) < 0.185)
          game.dropItem();
    }
    itemPriority();
	fine();
}
