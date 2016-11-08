float stato[12];
#define accelPerc 10 //percentage of space used to accelerate & decelerate
#define fMax 0.46
class{
    public:
    bool targetReached;
    float target[3];
    float acc;//max acceleration
    float distTot;//total distance from target
    float x;//acceleration & braking space
    void setV(float v[],float x,float y,float z)/*Definitivo*/{
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    void setV(float v[],float z[])/*Definitivo*/{
        v[0] = z[0];
        v[1] = z[1];
        v[2] = z[2];
    }
    float dist(float myPos[], float targetPos[]){
        float tempVec[3];
    	mathVecSubtract(tempVec,targetPos,myPos,3);
    	return (mathVecMagnitude(tempVec,3));
    }
    float massa(){
        float mult = 1+(game.getNumSPSHeld()/8);//sets the multiplier to 1(the sphere) +1/8(the weight of an SPS) *num of SPS held
        //if item held == LARGE --> mult+=3/8;
        //else if item held == MEDIUM --> mult+=2/8;
        //else if item held == SMALL --> mult+=1/8;
        return 4.15*mult;
    }//calculate the actual weight of the sphere
    void newTarg(float targ[],float stato[]){
        setV(target,targ);//copy target in the instance
        targetReached=false;
    	//get all the data
        distTot=dist(stato,target);
        acc=fMax/massa();
        x=(distTot/100)*accelPerc;
    	//print something to console
        DEBUG(("%f, %f",distTot,x));
    }//all ops to do when a new terget is set
    void everySec(float stato[]){
        DEBUG(("%f",dist(stato,target)));
        #define slowX 2
        if(dist(stato,target)<x/(5*slowX)){//target reached
            targetReached=true;
            DEBUG(("we've arrived, do what you need FAST!!"));
            api.setPositionTarget(stato);
        }
        else if(dist(stato,target)<x/slowX){//brake
            float vt[3];//temp vector used for various calculations
            setV(vt,0,0,0);
            api.setVelocityTarget(vt);
            DEBUG(("hey man, you don't want an accident to happen, SLOW DOWN, SLOW DOWN NOW!!"));
        }
        else if(dist(stato,target)>distTot-x){//accelerate
            float vt[3];//temp vector used for various calculations
            float vel[3];//velocity from ZRSPHERE state array
            setV(vel,stato[3],stato[4],stato[5]);//getting velocity vector
            mathVecSubtract(vt,target,stato,3);
            mathVecNormalize(vt,3);//direction to target
            setV(vt,vt[0]*acc,vt[1]*acc,vt[2]*acc);
            mathVecSubtract(vel,vt,vel,3);
            mathVecSubtract(vt,vt,vel,3);
            mathVecNormalize(vt,3);//direction to target
            setV(vt,vt[0]*acc,vt[1]*acc,vt[2]*acc);
            api.setVelocityTarget(vt);
            DEBUG(("GO GO GO MATE!!"));
        }
        //else --> moving by momentum
    }//call it as the last thing in loop(), just do it
}move;

void init(){
    float vai[3];
	api.getMyZRState(stato);
	move.setV(vai,stato[0]+0.5,stato[1],stato[2]);
	move.newTarg(vai,stato);
}

void loop(){
    float vai[3];
	api.getMyZRState(stato);
	if(api.getTime()>15 && move.targetReached){
	    move.setV(vai,stato[0],stato[1],stato[2]-0.5);
	    move.newTarg(vai,stato);
	}
	
	move.everySec(stato);
}
