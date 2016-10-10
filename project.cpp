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

float dist(float a[],float b[])/*Definitivo*/{
	return sqrt(mathSquare(a[0] - b[0]) + mathSquare(a[1] - b[1]) + mathSquare(a[2] - b[2]));
}

void setV(float v[],float x,float y,float z)/*Definitivo*/{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

bool oOB(float ptc[])/*Definitivo*/{
	return fabsf(ptc[0]) > 0.75 || fabsf(ptc[1]) > 0.75 || fabsf(ptc[2]) > 0.75;
}

void ruota()/*Definitivo*/{
	float v[3];
	mathVecSubtract(v,punta,pos,3);
	mathVecNormalize(v,3);
	api.setAttitudeTarget(v);
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
		if(d > 0.50)
			api.setVelocityTarget(vec);
		api.setPositionTarget(vai);
		if(d < 0.1)
			api.setPositionTarget(pos); //sostituisce frena())
	}
}

int checkDock(int id)/*boh*/{
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
	if(checkDock(id))/*docko*/{
		game.dockItem();
	}
}

void init(){
	for(int i = 0; i < 3; i++)vai[i] = punta[i] = 0;
	api.getMyZRState(stato);
	BoR = stato[1] > 0 ? 'R' : 'B'; //capisco quale sfera siamo
	fase = sottofase = 0;
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

void loop(){
    inizio();
	switch(fase){
        case 0://drop SPSs, i cosi per sapere dove si trova la zhohona
            if (sottofase==0 && dist(vai,pos)<0.1){
                game.dropSPS();
                sottofase++;
                setV(vai, -0.4, -0.5, 0);
                //game.getItemLoc(vai,1);
            }
            else if (sottofase==1 && dist(vai,pos)<0.1){
                game.dropSPS();
                sottofase++;
                setV(vai, -0.2, 0.3, 0);
                //game.getItemLoc(vai,5);
            }
            else if(sottofase==2 && dist(vai,pos)<0.1){
                game.dropSPS();
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
	fine();
}
