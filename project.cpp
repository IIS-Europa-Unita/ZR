char BoR; // R se sfera rossa, B se blu
int fase,sottofase,ID;
float stato[12];
float statoAvv[12];
float vai[3];
float posAvv[3];
float facing[3];
float vel[3];
float speed;
float zona[4];
float v[3];
bool LARGE, MEDIUM, SMALL;
bool spsAllDrop;

float dist(float a[],float b[])/*Definitivo*/{
	return sqrt(mathSquare(a[0] - b[0]) + mathSquare(a[1] - b[1]) + mathSquare(a[2] - b[2]));
}

void setV(float v[],float x,float y,float z)/*Definitivo*/{
	v[0] = x;v[1] = y;v[2] = z;
}

bool oOB()/*Definitivo*/{
	if ((fabsf(stato[0]) > 0.75) ||
	    (fabsf(stato[1]) > 0.75) ||
	    (fabsf(stato[2]) > 0.75))
	    return true;
	return false;
}

void ruota(){
	if(spsAllDrop && dist(stato,vai) < 0.5){//per risparmiare fuel
	  mathVecSubtract(v,vai,stato,3);//uso vai
	  mathVecNormalize(v,3);
	  api.setAttitudeTarget(v);
	}
}

void muovi(){
	if(oOB()){setV(vai,0,0,0);api.setPositionTarget(vai); DEBUG(("oob!!"));}
	else{
		float d = dist(vai,stato);
		DEBUG(("%f",d));
		mathVecSubtract(v,vai,stato,3);
		if(spsAllDrop){
		      if(d > 0.267)
		        api.setPositionTarget(vai);
		      else 
		        api.setPositionTarget(stato);
		}
		else{
		    if(d > 0.50)
			    api.setVelocityTarget(v);
		    api.setPositionTarget(vai);
		    if(d < 0.13)
			    api.setPositionTarget(stato);
        }
	}
}

int checkDock()/*verifico condizioni per docking*/{
	game.getItemLoc(v,ID); //Posizione dell'oggetto
	float targD = dist(v,stato); //Distanza dall'oggetto 
	if(((ID == 0 || ID == 1) && targD < 0.151) ||
	((ID == 2 || ID == 3) && targD < 0.138) ||
	((ID == 4 || ID == 5) && targD < 0.124)){
		DEBUG(("Vicini"));
		return 1;
	}
	if(((ID == 0 || ID == 1) && targD > 0.173) ||
	((ID == 2 || ID == 3) && targD > 0.160) ||
	((ID == 4 || ID == 5) && targD > 0.146)){
		DEBUG(("Lontani"));
		return 2;
	}
	if(speed > 0.01){
		DEBUG(("Vel"));
		return 3;
	}
	return 0;
}

void dock(){
    switch (checkDock()){
        case 0:
          game.dockItem();
          if(game.hasItemBeenPickedUp(ID) || game.hasItemBeenPickedUp(ID+1))
            switch (ID){
                case 0: case 1: LARGE = FALSE; break;
                case 2: case 3: MEDIUM = FALSE; break;
                case 4: case 5: SMALL = FALSE;
            }
        break;
        case 1:
            setV(vai,vai[0]+(vai[0]>0)?-0.03:0.03,vai[1]+
                (vai[1]>0)?-0.03:0.03,vai[2]+(vai[2]>0)?-0.03:0.03);
        break;
        case 2:
            setV(vai,vai[0]+(vai[0]>0)?0.03:-0.03,vai[1]+
                (vai[1]>0)?0.03:-0.03,vai[2]+(vai[2]>0)?0.03:-0.03);
        break;
        case 3:
          api.setPositionTarget(stato);
    }
}

void init(){
	setV(vai,0,0,0);
	api.getMyZRState(stato);
	BoR = stato[1] > 0 ? 'R' : 'B'; //capisco quale sfera siamo
	fase = sottofase = ID = 0;
	spsAllDrop = false;
	LARGE = MEDIUM = SMALL = true;
}

void itemPriority(){
    /*float large[3]={0.375, 0.727, 0.2}; //massa, accelerazione, punteggio
    float medium[3]={0.25, 0.8, 0.15};
    float small[3]={0.125, 0.889, 0.1};*/
    float d1[3];
    if(!LARGE)
      ID=2;//0-1 sono i large, 2-3 i medium e 4-5 gli small, per quello incremento i così
    if(!MEDIUM)
      ID=4;
    if(!SMALL){
      MEDIUM = true;//se ho preso anche uno small, passo di nuovo al medium e poi all'altro small
      ID=2;
    }
    game.getItemLoc(v,ID);
    game.getItemLoc(d1,(ID+1));
    if((dist(stato, v)+dist(v, zona)) > (dist(stato, d1)+dist(d1, zona))
    && !game.hasItemBeenPickedUp(ID+1))
        //restituisce l'id dell'item più conveniente da andare a prendere
        ID++;
}

void loop(){
    api.getMyZRState(stato); //aggiorno dati miei
	api.getOtherZRState(statoAvv); //aggiorno dati avversario
	itemPriority();
	for(int i = 0; i < 3; i++){
		facing[i] = stato[i + 6];
		posAvv[i] = statoAvv[i];
		vel[i] = stato[i + 3];
	}//ottengo pos + dir miei e pos avversario
	speed = mathVecMagnitude(vel,3);
	switch(fase){
        case 0://drop SPSs, i cosi per sapere dove si trova la zhohona
            if (sottofase==0){
                game.dropSPS();
                sottofase++;
                setV(vai, -0.3, -0.5, -0.25);
                //game.getItemLoc(vai,1);
            }
            else if (sottofase==1 && dist(vai,stato)<0.1){
                game.dropSPS();
                sottofase++;
                setV(vai, -0.3, 0.3, 0.25                                               );
                //game.getItemLoc(vai,5);
            }
            else if(sottofase==2 && dist(vai,stato)<0.1){
                game.dropSPS();
                spsAllDrop = true;
                sottofase=0;
                fase++;
                itemPriority();
            }
        break;
        case 1://getZone
            if(!game.getZone(zona))//se true, carica anche la posizione della sfera in zona
                fase--;
            if(game.hasItem(ID) == 0){
                itemPriority();
                DEBUG (("ID: %d", ID));
                game.getItemLoc(vai, ID);
                dock();
            }
            else{
                setV(vai, zona[0],zona[1],zona[2]);
                if(dist(stato,vai) < 0.185) game.dropItem();
            }
        break;
        default:
        DEBUG(("ERROR"));
	}
	DEBUG(("%f, %f, %f",vai[0],vai[1],vai[2]));
	ruota(); //rotazione verso punta[]
	muovi(); //spostamento verso vai[]
}
