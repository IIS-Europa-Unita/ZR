float stato[12];
float vai[3];
float forza,amax,distTot,x;
float y,vb[3];
float vel[3];
float vInerzia,ts,tTot,tInerzia;
bool once,onceB;
float d,dprec;
float dist(float a[],float b[])/*Definitivo*/{
	return sqrt(mathSquare(a[0] - b[0]) + mathSquare(a[1] - b[1]) + mathSquare(a[2] - b[2]));
}
#define tempo api.getTime()
void setV(float v[],float x,float y,float z)/*Definitivo*/{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void init(){
    d=10;
	api.getMyZRState(stato);
	vai[0]=stato[0]+0.7;
	vai[1]=stato[1];
	vai[2]=stato[2]-0.7;
	forza=0.01;
	amax=forza/4.15;
	distTot=dist(vai,stato);
	DEBUG(("%f",distTot));
	x=(distTot/100)*20;
	y=distTot-(2*x);
	ts=sqrt(2*x/amax);
	vInerzia=amax*ts;
	tInerzia=y/vInerzia;
	tTot=tInerzia+2*ts;
	DEBUG(("%f, %f\n %f, %f, %f",x,y,ts,tInerzia,tTot));
	once=onceB=true;
}

void loop(){
	api.getMyZRState(stato);
	setV(vel,stato[3],stato[4],stato[5]);//ottengo vettore velocità
	mathVecMagnitude(vel,3);
	dprec=d;
	d=dist(vai,stato);
	DEBUG(("dist= %f",d));
	mathVecSubtract(vb,vai,stato,3);
	mathVecNormalize(vb,3);
	setV(vb,vb[0]*forza,vb[1]*forza,vb[2]*forza);//ottengo forza con direzione
	if(tempo<=ts){
	    api.setForces(vb);
	}
	else if(once){
	    once=false;
	    DEBUG(("OFF"));
	}
	if(dprec<d&&onceB){
	    onceB=false;
	    DEBUG(("arrivato, %f",d));
	}
	if(tempo>ts+tInerzia && tempo<tTot){
	    setV(vb,vb[0]*-1,vb[1]*-1,vb[2]*-1);
	    api.setForces(vb);
	}
	//*/api.setPositionTarget(vai);
}
