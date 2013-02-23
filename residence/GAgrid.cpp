#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<conio.h>
#include<math.h>
#include<time.h>
#include<direct.h>

int CASE = 1;
const int MAXDU = 3;	//maximum working duration
const int MTYPE = 14;	//number of machine types
const int MAXDELAY = 12;//max. hours of delay
const int TGN = 210;	//gene number of each type
const int GENN = TGN * MTYPE;
const int POPSIZE = 200;
const int NSEL = 10;		// no. for tournament selection
const int TRIALNUM = 50;
const int MAXGENS = 5000;
double PX=0.9;//0.85;					// crossover rate
double PM=0.1;					// mutation rate

int Mnum[MTYPE];		//number of machines of the type
int Mdurn[MTYPE];		//number of duration hour for each type
double Mdur[MTYPE][MAXDU];//energy consumption during the working duration
double eprice[24];		//price of energy in 24 hours
double uenergy[24];		//used energy of the GROUP
int matrix[23][12];

int iMnum[24][MTYPE];	//number of machines each type per hour in the new instance
int itMnum[24][MTYPE];	//temp number of machines each type per hour in the new instance
double iushenergy[24];	//unshiftable energy used in the new instance
double ishenergy[24];	//shiftable energy used in the new instance
double shiftsum = 0;	//consumption of shiftable machines
double unshisum = 0;	//consumption of unshiftable machines
double objcurve[24];	//objective curve of energy consumption
int trialNum;
FILE *fin,*foutinst,*ftotal,*fresultgMnum,*fruntime;
char str[100],ch[10],strdir[100];
clock_t avg_result_time;

struct CHROMOSOME {
	int gene[GENN];
	int gMnum[24][MTYPE];	//该时刻上启动的设备数
	int gsMnum[24][MTYPE];  //该时刻去掉已移走的剩下的原可移动设备数
	double fitness;
	double cost;
} pop[POPSIZE+1], npop[POPSIZE+1];

void generateInst(int inst) {
	int i,j,k;
	double temp,temp2;
	double ran[24];

	fin = fopen("resitype.txt","r");
	shiftsum = 0;
	for(i=0;i<MTYPE;i++) {
		k = 0;
		for(j=0;j<MAXDU;j++) {
			fscanf(fin,"%lf",&Mdur[i][j]);
			printf("%g\t",Mdur[i][j]);
			if(Mdur[i][j] > 0)
				k++;
		}
		Mdurn[i] = k;
		fscanf(fin,"%d",&Mnum[i]);
		printf("%d\n",Mnum[i]);
		for(j=0;j<MAXDU;j++)
			shiftsum += Mdur[i][j] * Mnum[i];
	}
	printf("%g\n",shiftsum);
	fclose(fin);

	fin = fopen("pricedemand.txt","r");
	unshisum = 0;
	for(i=0;i<24;i++) {
		fscanf(fin,"%d%lf%lf%lf%lf",&k,&eprice[i],&uenergy[i],&temp,&temp);//residence!!change here
		unshisum += uenergy[i];
	}
	unshisum -= shiftsum;
	printf("%.13g\n",unshisum);
	fclose(fin);

	//randomly distribute the start time of shiftable machines
	for(j=0;j<MTYPE;j++) {
		k = Mnum[j];	//number of machines of the type
		temp2 = 0;
		for(i=0;i<(24-Mdurn[j]+1);i++) {
			ran[i] = rand()%100/100.0;
			temp2 += ran[i];
		}
		for(i=0;i<(24-Mdurn[j]);i++) {	
			iMnum[i][j] = int(Mnum[j] * ran[i]/temp2);	//choose from the rest machines
			k -= iMnum[i][j];	//update the number of rest machines
		}
		iMnum[i][j] = k;	//the rest are for the last feasible hour
		for(i++;i<24;i++) 
			iMnum[i][j] = 0;
	}
	//compute shiftable consumption for each hour
	for(i=0;i<24;i++)
		ishenergy[i] = 0;
	for(j=0;j<MTYPE;j++) {
		for(i=0;i<(24-Mdurn[j]+1);i++) {
			for(k=0;k<Mdurn[j];k++) {
				ishenergy[i+k] += Mdur[j][k] * iMnum[i][j];
			}
		}
	}
	//distribute unshiftable energy consumption in each hour
	for(i=0;i<24;i++) {
		iushenergy[i] = uenergy[i] - ishenergy[i];
		if(iushenergy[i]<=0) { 
			printf("need to rerun the instance\n");
			getch();
		}
	}

	itoa(inst,ch,10);
	strcpy(str,ch);
	strcat(str,"inst.txt");
	foutinst = fopen(str,"w");
	for(i=0;i<24;i++) 
		fprintf(foutinst,"%d\t%g\t%.13g\t%.13g\n",i,eprice[i],ishenergy[i],iushenergy[i]);
	fclose(foutinst);

	itoa(inst,ch,10);
	strcpy(str,ch);
	strcat(str,"iMnum.txt");
	foutinst = fopen(str,"w");
	for(i=0;i<24;i++) {
		for(j=0;j<MTYPE;j++)
			fprintf(foutinst,"%d\t",iMnum[i][j]);
		fprintf(foutinst,"\n");
	}
	fclose(foutinst);
}

void readInst(int inst) {
	int i,j,k;
	double temp;

	itoa(inst,ch,10);
	strcpy(str,ch);
	strcat(str,"inst.txt");
	fin = fopen(str,"r");
	for(i=0;i<24;i++) {
		fscanf(fin,"%d%lf%lf%lf",&j,&eprice[i],&ishenergy[i],&iushenergy[i]);
		uenergy[i] = ishenergy[i] + iushenergy[i];
	}
	fclose(fin);

	fin = fopen("resitype.txt","r");
	shiftsum = 0;
	for(i=0;i<MTYPE;i++) {
		k = 0;
		for(j=0;j<MAXDU;j++) {
			fscanf(fin,"%lf",&Mdur[i][j]);
			if(Mdur[i][j] > 0)
				k++;
		}
		Mdurn[i] = k;
		fscanf(fin,"%d",&Mnum[i]);
		for(j=0;j<MAXDU;j++)
			shiftsum += Mdur[i][j] * Mnum[i];
	}
	fclose(fin);

	fin = fopen("pricedemand.txt","r");
	unshisum = 0;
	for(i=0;i<24;i++) {
		fscanf(fin,"%d%lf%lf%lf%lf",&k,&eprice[i],&uenergy[i],&temp,&temp);//residence!!change here
		unshisum += uenergy[i];
	}
	unshisum -= shiftsum;
	fclose(fin);

	itoa(inst,ch,10);
	strcpy(str,ch);
	strcat(str,"iMnum.txt");
	fin = fopen(str,"r");
	for(i=0;i<24;i++) {
		for(j=0;j<MTYPE;j++)
			fscanf(fin,"%d",&iMnum[i][j]);
		fscanf(fin,"\n");
	}
	fclose(fin);
}

int min(int a,int b) {
	if(a<b) return a;
	return b;
}

int max(int a,int b) {
	if(a>b) return a;
	return b;
}

void initialize(int inst) {
	int i,j,k,t,l;
	double sum1 = 0;
	double sum2 = 0;
	for(i=0;i<24;i++) {
		sum1 += uenergy[i];
		sum2 += 1.0 / eprice[i];
	}
	for(i=0;i<24;i++) 
		objcurve[i] = 1.0 / eprice[i] / sum2 * sum1;

	//initialize population
	for(j=0; j<POPSIZE; j++) {
		for(t=0; t<24; t++) {
			for(k=0; k<MTYPE; k++)
				pop[j].gMnum[t][k] = pop[j].gsMnum[t][k] = itMnum[t][k] = iMnum[t][k];			//copy the number of machines for each hour and each type
		}
		for(i=0; i<GENN; i++)
			pop[j].gene[i] = 0;
//		foutinst = fopen("checkshift.txt","w");
		for(k=0; k<MTYPE; k++) {
//			fprintf(foutinst,"type = %d\n",k);
			l = TGN*k;		//index for the genes
			for(i=0; i<MAXDELAY; i++) {
				for(t=0; t<MAXDELAY; t++) {
					if((i+t+1)<(24-Mdurn[k]+1)) {
						if(itMnum[i][k] > 0) {	//if it is 0, then no machine can be shifted
							pop[j].gene[l] = rand() % (itMnum[i][k]+1);
							if(pop[j].gene[l] != 0) {
								pop[j].gMnum[i][k] -= pop[j].gene[l];	//shift from i to i+t+1
								pop[j].gsMnum[i][k] -= pop[j].gene[l];	//shift from i to i+t+1
								itMnum[i][k] -= pop[j].gene[l];
								pop[j].gMnum[i+t+1][k] += pop[j].gene[l];
//								fprintf(foutinst,"%d to %d with %d\n",i,i+t+1,pop[j].gene[l]);
							}
						}
					}
					l++;
				}
			}
			for(; i<23; i++) {
				for(t=0; t<(MAXDELAY-i+MAXDELAY-1); t++) {
					if((i+t+1)<(24-Mdurn[k]+1)) {
						if(itMnum[i][k] > 0) {
							pop[j].gene[l] = rand() % (itMnum[i][k]+1);
							if(pop[j].gene[l] != 0) {
								pop[j].gMnum[i][k] -= pop[j].gene[l];	//shift from i to i+t+1
								pop[j].gsMnum[i][k] -= pop[j].gene[l];	//shift from i to i+t+1
								itMnum[i][k] -= pop[j].gene[l];
								pop[j].gMnum[i+t+1][k] += pop[j].gene[l];
//								fprintf(foutinst,"%d to %d with %d\n",i,i+t+1,pop[j].gene[l]);
							}
						}
					}
					l++;
				}
			}	
		}
//		fclose(foutinst);

/*		itoa(inst,ch,10);
		strcpy(str,ch);
		strcat(str,"outcheckgNnum1.txt");
		foutinst = fopen(str,"w");
		for(t=0; t<24; t++) {
			for(k=0; k<MTYPE; k++) {
				fprintf(foutinst,"%d\t",pop[j].gMnum[t][k]);
			}
			fprintf(foutinst,"\n");
		}
		fclose(foutinst);

		itoa(inst,ch,10);
		strcpy(str,ch);
		strcat(str,"outcheckgene.txt");
		foutinst = fopen(str,"w");
		for(i=0;i<GENN;i++) {
			fprintf(foutinst,"%d\t",pop[j].gene[i]);
		}
		fclose(foutinst);*/
	}
}

void evaluate(int inst) {
	int i,j,k,t,l,q;
	double sum,sum1[24];

	for(j=0; j<POPSIZE; j++) {
		for(t=0; t<24; t++) {
			for(k=0; k<MTYPE; k++)
				pop[j].gMnum[t][k] = pop[j].gsMnum[t][k] = iMnum[t][k];			//copy the number of machines for each hour and each type
		}
		for(t=0; t<24; t++)
			sum1[t] = 0;
		for(k=0; k<MTYPE; k++) {
			for(i=0; i<23; i++) {
				for(t=0; t<12; t++)
					matrix[i][t] = 0;	//initialize the matrix
			}
			l = TGN*k;		//index for the genes
//			foutinst = fopen("checkmatrix.txt","w");
			for(i=0; i<MAXDELAY; i++) {
				for(t=0; t<MAXDELAY; t++) {
					matrix[i][t] = pop[j].gene[l];
//					fprintf(foutinst,"%d\t",matrix[i][t]);
					l++;
				}
//				fprintf(foutinst,"\n");
			}
			for(; i<23; i++) {
				for(t=0; t<(MAXDELAY-i+MAXDELAY-1); t++) {
					matrix[i][t] = pop[j].gene[l];
//					fprintf(foutinst,"%d\t",matrix[i][t]);
					l++;
				}
//				fprintf(foutinst,"\n");
			}
//			fclose(foutinst);
			//use the equations to compute
			for(t=0; t<24; t++) {
				//connect(t)
				for(i=max(0,t-MAXDELAY); i<t && t<(24-Mdurn[k]+1); i++) {
					sum1[t] += matrix[i][t-i-1] * Mdur[k][0];
					pop[j].gMnum[i][k] -= matrix[i][t-i-1];	//the start time of machines are delayed to t
					pop[j].gsMnum[i][k] -= matrix[i][t-i-1];	//the start time of machines are delayed to t
					pop[j].gMnum[t][k] += matrix[i][t-i-1];
					if(pop[j].gMnum[i][k] <0) {
						printf("error\n");
						getch();
					}
				}
				for(l=1; l<Mdurn[k]; l++) {
					for(i=max(0,t-MAXDELAY-l); i<(t-l-1); i++) {
						sum1[t] += matrix[i][t-i-1 -l] * Mdur[k][l];
					}
				}
				//disconnect(t)
				q = min(t+MAXDELAY, (24-Mdurn[k]+1)-1) - t -1;
				for(i=0; i<=q; i++)
					sum1[t] -= matrix[t][i] * Mdur[k][0];
				for(l=1; l<Mdurn[k]; l++) {
					if( (t-l)>=0 ) {
						for(q=t-l+1 - t; q<=(MAXDELAY-l); q++) {
							sum1[t] -= matrix[t-l][q] * Mdur[k][l];
						}
					}
				}
			}
		}
		sum = 0;
		for(t=0; t<24; t++) 
			sum += pow(sum1[t] + uenergy[t] - objcurve[t], 2);
		pop[j].fitness = 1.0 / (1 + sum);

/*		itoa(inst,ch,10);
		strcpy(str,ch);
		strcat(str,"outcheckgNnum2.txt");
		foutinst = fopen(str,"w");
		for(t=0; t<24; t++) {
			for(k=0; k<MTYPE; k++) {
				fprintf(foutinst,"%d\t",pop[j].gMnum[t][k]);
			}
			fprintf(foutinst,"\n");
		}
		fclose(foutinst);*/

		//compute shiftable consumption for each hour
		for(i=0;i<24;i++)
			ishenergy[i] = 0;
		for(t=0;t<MTYPE;t++) {
			for(i=0;i<(24-Mdurn[t]+1);i++) {
				for(k=0;k<Mdurn[t];k++) {
					ishenergy[i+k] += Mdur[t][k] * pop[j].gMnum[i][t];
				}
			}
		}

	/*	foutinst = fopen("tempfile.txt","w");
		for(i=0;i<24;i++) {
			fprintf(foutinst,"%d\t%g\t%g\t%g\n",i,ishenergy[i], iushenergy[i],ishenergy[i] + iushenergy[i]);
		}
		fprintf(foutinst,"fitness = %g\n",pop[j].fitness);
		fclose(foutinst);*/

		pop[j].cost = 0;
		for(i=0;i<24;i++)
			pop[j].cost += eprice[i] * (ishenergy[i] + iushenergy[i]);
	}			
}

void keep_the_best()
{
	int i,j = 0;					// stores the index of the best individual
	double best=pop[0].fitness;       

	for(i=1; i<POPSIZE; i++) {
		if(pop[i].fitness > best) {
			j = i;
			best = pop[i].fitness;
		}
	}

	//record the best individual in pop[POPSIZE]
	pop[POPSIZE] = pop[j];
}

void select() { //tournament selection
	int i, j;
	int child[NSEL], temp;
	double max;
	for(i=0; i<POPSIZE; i++) {
		for(j=0;j<NSEL;j++) 
			child[j] = rand()%POPSIZE;

		max = pop[child[0]].fitness;
		temp = child[0];
		for(j=1; j<NSEL; j++) {
			if(max<pop[child[j]].fitness) {
				max=pop[child[j]].fitness;
				temp=child[j];
			}
		}
		npop[i] = pop[temp];
	}
	
	for(i=0; i<POPSIZE; i++)
		pop[i] = npop[i]; 
}

void Xover(int one, int two)
{
	int i;
	int point; // crossover point 
	int tf;
	
	// select crossover point 
	if(GENN > 1) {
		if(GENN == 2)
			point = 1;
		else
			point =(rand()%(GENN-1)) + 1;
		
		for(i=0; i<point; i++) {
			tf = pop[one].gene[i];
			pop[one].gene[i] = pop[two].gene[i];
			pop[two].gene[i] = tf;
		}
	}
}

void adjust(int pn) {//adjust infeasible solutions pop[pn] to be feasible
	int i,k,t,l,u,ran,sl;
	int sum1;

    //check for adjustment
	for(k=0; k<MTYPE; k++) {
		l = TGN*k;		//index for the genes
		for(i=0; i<MAXDELAY; i++) {
			sum1 = 0;
			sl = l;
			for(t=0; t<MAXDELAY; t++) {
				if((i+t+1)<(24-Mdurn[k]+1))
					sum1 += pop[pn].gene[l];
				l++;
			}
			while(sum1 > iMnum[i][k]) {
				//randomly select one gene (>0) to reduce 1
				ran = rand() % MAXDELAY;//(24-Mdurn[k]+1 - i);
				while((i+ran+1)<(24-Mdurn[k]) && pop[pn].gene[sl+ran] == 0) ran++;
				if(ran<MAXDELAY && (i+ran)<(24-Mdurn[k]) && pop[pn].gene[sl+ran] > 0) {	//经测试，这里要ran几次才成功
					pop[pn].gene[sl+ran]--;
					sum1--;
				}
			}
			pop[pn].gMnum[i][k] = iMnum[i][k] - sum1;
			pop[pn].gsMnum[i][k] = iMnum[i][k] - sum1;
		}
		for(; i<23; i++) {
			sum1 = 0;
			sl = l;
			u = MAXDELAY-i+MAXDELAY-1;
			for(t=0; t< u; t++) {
				if((i+t+1)<(24-Mdurn[k]+1)) 
					sum1 += pop[pn].gene[l];
				l++;
			}
			while(sum1 > iMnum[i][k]) {
				//randomly select one gene (>0) to reduce 1
				ran = rand() % (24-Mdurn[k] - i);
				while((ran+1)<(24-Mdurn[k] - i) && pop[pn].gene[sl+ran] == 0) ran++;
				if(ran<(24-Mdurn[k] - i) && pop[pn].gene[sl+ran] > 0) {
					pop[pn].gene[sl+ran]--;
					sum1--;
				}
			}
			pop[pn].gMnum[i][k] = iMnum[i][k] - sum1;
			pop[pn].gsMnum[i][k] = iMnum[i][k] - sum1;
		}
	}
}

void crossover() { //one point crossover
	int j, one;
	int first = 0; // count of the number of members chosen 
	double x;
	
	for(j=0; j<POPSIZE; j++) {
		x = rand()/(double)RAND_MAX;
		if(x < PX) {
            first++;
            if(first%2 == 0) {
				Xover(one, j);	//crossover
				adjust(one);
				adjust(j);
			}
            else
				one = j;
		}
	}
}

void mutate() {
	int i, j, k, l, t, u;
	double x;	

	for(i=0; i<POPSIZE; i++) { 	
		for(j=0; j<GENN; j++) {
			x = rand()/(double)RAND_MAX;
			if(x < PM) {
				k = j / TGN;	//which type
				l = j % TGN;	//location in the type
				for(t=0; t<MAXDELAY; t++) {
					if((l-MAXDELAY) < 0) 
						break;
					l -= MAXDELAY;	//move to the next time step
				}
				if(t == MAXDELAY) {
					for(; t<23; t++) {
						u = (MAXDELAY-t+MAXDELAY-1);
						if((l-u) < 0) 
							break;
						l -= u;
					}
				}
				//randomly select a feasible number
				u = pop[i].gene[j]+pop[i].gsMnum[t][k];
				l = pop[i].gene[j];
				if(u > 0) {
					pop[i].gene[j] = rand() % (u+1);
					pop[i].gsMnum[t][k] = u - pop[i].gene[j]; //update 
					pop[i].gMnum[t][k] += l - pop[i].gene[j]; //update
				}
			}
		}
	}
}

//if a better individual is found, store it as best-so-far (pop[POPSIZE])
//else replace the worst individual with the best-so-far (pop[POPSIZE])
void elitist() {
	int i;
	double best, worst;             // best and worst fitness values 
	int best_i, worst_i;		// indexes of the best and worst member 
	
	best = pop[0].fitness;
	worst = pop[0].fitness;
	for(i=1; i<POPSIZE; i++) {
		if(pop[i-1].fitness > pop[i].fitness) {      
            if(pop[i-1].fitness >= best) {
				best = pop[i-1].fitness;
				best_i = i-1;
			}
            if (pop[i].fitness <= worst) {
				worst = pop[i].fitness;
				worst_i = i;
			}
		}
		else {
            if (pop[i-1].fitness <= worst) {
				worst = pop[i-1].fitness;
				worst_i = i-1;
			}
            if (pop[i].fitness >= best) {
				best = pop[i].fitness;
				best_i = i;
			}
		}
	}
	
	if (best > pop[POPSIZE].fitness) { //a better result is found
		pop[POPSIZE] = pop[best_i];
	}
	else { //no better result is found	
		pop[worst_i] = pop[POPSIZE];
    } 
}

int main() {
//	srand(time(0));
//	generateInst(CASE);

	for(CASE = 1; CASE<=10; CASE++) {
		_mkdir("GAgrid");
		strcpy(strdir,"GAgrid//inst");
		itoa(CASE,ch,10);
		strcat(strdir,ch);
		_mkdir(strdir);
		strcat(strdir,"//");
	readInst(CASE);

	int i,j;
	int gen;
	clock_t start;			//////////////////
	avg_result_time = 0;	//////////////////
	for(trialNum = 1; trialNum <= TRIALNUM; trialNum++) {
		strcpy(str,strdir);
		strcat(str,"result.txt");
		ftotal = fopen(str,"a");

		strcpy(str,strdir);
		itoa(trialNum,ch,10);
		strcat(str,"resultMnum");
		strcat(str,ch);
		strcat(str,".txt");
		fresultgMnum = fopen(str,"w");

		strcpy(str,strdir);
		strcat(str,"runtime");
		strcat(str,ch);
		strcat(str,".txt");
		fruntime = fopen(str,"w");

		start = clock();	//////////////////
		gen = 0;
		initialize(CASE);
		evaluate(CASE);
		keep_the_best();
		while(gen < MAXGENS) {
			select();			//using roulette wheel selection
			crossover();
			mutate();
			evaluate(CASE);	
			elitist();						
			gen++;
		//	printf("%d\t%g\t%g\n",gen,pop[POPSIZE].fitness,pop[POPSIZE].cost);
			fprintf(fruntime,"%d\t%g\t%g\n",gen,pop[POPSIZE].fitness,pop[POPSIZE].cost);
		}
		avg_result_time += clock() - start; //////////////////
		printf("%d\t%g\t%g\n",trialNum,pop[POPSIZE].fitness,pop[POPSIZE].cost);
		fprintf(ftotal,"%d\t%g\t%g\t%g\n",trialNum,pop[POPSIZE].fitness,pop[POPSIZE].cost,(double)avg_result_time/trialNum);//////////////////
		for(i=0;i<24;i++) {
			for(j=0;j<MTYPE;j++) {
				fprintf(fresultgMnum,"%d\t",pop[POPSIZE].gMnum[i][j]);
			}
			fprintf(fresultgMnum,"\n");
		}
	//	printf("\n");
		fprintf(fresultgMnum,"\n");
		fclose(fresultgMnum);
		fclose(ftotal);
		fclose(fruntime);
	}
	}

	return 0;
}