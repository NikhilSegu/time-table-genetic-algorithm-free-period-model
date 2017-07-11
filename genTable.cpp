/*	
	Gen_Table.cpp
	
	Time Table generator (Lite) using Genetic Algorithm.
   ============================================================================
	Created and designed by Rupal Barman.
	Last modified on 14/09/2016
	rupalbarman@gmail.com 
*/
// ==============================================================================================

/*	This Variant of TimeTable generator has only 5 organisms per population/generation.
	The current generation and next generation is taken into consideration.
	The single combined organisms in current and next generation has 'neededOrgs * 30 genes'
	ie. if we have 3 organisms to mutate and comapre, the single combined 
	enitiy will have 30*3 genes. With those 30 genes again split up as 5 days * 6 subject/allele.
	This is done in order to establish a unique allele trait in those 3 organisms 
	(which is combined into a single one) with the free period model enitity.
*/

// ============================================================================
//	Includes
// ============================================================================

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>

// ============================================================================
//	Constants
// ============================================================================

int allele=6;
int days=5;
int orgs=5; //organisms in one generation
int genes=30; //allele * days
int generations=100;
int neededOrgs=3;	//no. of sections needed
double mutationRate=0.01;

using namespace std;

// ============================================================================
//	Global variables
// ============================================================================

int **curG,**nextG,*model;
int *fitness;
int **elitsm;
int eliteCount=0;
int maxFit=0;
int maxOrganismIndex;

//Input/ Output text file

ofstream table("Table.txt");
ifstream model_file("model.txt");

// ============================================================================
//	Function prototypes.
// ============================================================================

void subjectGuide(); //Shows the subject names associated with each value.
void memAllocate(); //Initializes all data elements, populations and others.
void getModel(); //Initializes the model with a predefined set of values.
void getCustomModel(); // Gets a custom set of values from the user for model to be used.
void whichModel(); // Decides which model to be used (based on user preferences).
void showModel(int*); //Helper function to display the model which the results are computed (evolved) against.
void fitnessCalc(); //Finds the fitness of the organisms in the current generation based on the model values.
void showFitness(); //Helper function to display the overall fitness of organisms.
void showPopulation(); //Helper function to display the current generation organism's genes/values.
void initPopulation(); //Initializes the current population with alleles.
int getBestParent(int); //Selects two best (highed fitness) parents for breeding (crossover).
int getRandomParent(); // Select two random parents from a generation for breeding.
void newGeneration();  //Main mutation function which determins mutation and crossover (creation of new generation).
void getElitsm(); //Keeps track of the best fit organism in a best overall fitted generation.
void getRandomModel(); //initializes model with random allele values
void getModelFromFile(); //gets model from a file named "model.txt"
int rouletteWheelSelection(int); //Each parent with highest fitness have equal chance of selecting.


// ============================================================================
//	Subject Parser class defination
// ============================================================================

class SubjectParser
{
	int subjects;
public:
	SubjectParser()
	{
		subjects=6;
	}
	string subjectNameFinder(int subValue)
    {
        string sub="";
        switch(subValue)
        {

            case 1: sub="Math"; break;
            case 2: sub="Phy"; break;
            case 3: sub="Chem"; break;
            case 4: sub="Comp"; break;
            case 5: sub="Bio"; break;
            case 6: sub="Sport"; break;
        }
        return sub;
    }

    //Writes the free period model to the start of the file table.txt
    void writeFreePeriodModel()
    {
    	table<<"Free Period Model"<<endl;
    	int day=1;
        for(int k=1;k<=genes;k++)
        		{

        			if((k-1)%subjects==0)
        			{
                		table<<"\n\t Day: "<<day;
                		day++;
                	}

            		else
            		{
            			table<<"\t"<<subjectNameFinder(model[k-1]);
            		}
        		}
        		table<<endl;
        table<<"======================================================\n";
    }
    
    //Main time table display function which assigns the subjects to its respective
    //values and writes to the console and to the file table.txt.
    //also calls the free period model function.
    void generateTimeTable(int **elite)
    {
        //First write the free period model to file
        writeFreePeriodModel();

        table<<"Time Table\n";

        int day=1;
        int k=1;
        for(int j=1;j<=neededOrgs*genes;j+=genes)
        	{
        		for(int k=1;k<=genes;k++)
        		{

        			if((k-1)%subjects==0)
        			{
                		cout<<"\n\t Day "<<day;
                		table<<"\n\t Day: "<<day;
                		day++;
                	}

            		else
            		{
            			cout<<"\t"<<subjectNameFinder(elite[maxOrganismIndex][j+k-2]);
            			table<<"\t"<<subjectNameFinder(elite[maxOrganismIndex][j+k-2]);
            		}
        		}
        		day=1;
        		cout<<endl;
        		table<<endl;
        	
        	cout<<endl;
        	table<<endl;
        }
    }

}parse;

// ============================================================================
//	Functions
// ============================================================================

void subjectGuide()
{
	cout<<endl;
	cout<<"1.Math\t\t2.Physics\t\t3.Chemistry\n4.Computer\t5.Biology\t\t6.Sports\n";
}

void memAllocate()
{
	curG=(int**)malloc(sizeof(int)*orgs);
    nextG=(int**)malloc(sizeof(int)*orgs);
    fitness=(int*)malloc(sizeof(int)*orgs);
    model=(int*)malloc(sizeof(int)*genes*neededOrgs);
    elitsm=(int**)malloc(sizeof(int)*orgs);

    for(int i=0;i<orgs;i++)
    {
        curG[i]=(int*)malloc(sizeof(int)*genes*neededOrgs);
        nextG[i]=(int*)malloc(sizeof(int)*genes*neededOrgs);
        elitsm[i]=(int*)malloc(sizeof(int)*genes*neededOrgs);
    }
}

//Make sure file model.txt already exists, or it switches to custom model
//also in file, enter values seperated by a space or newline
void getModelFromFile()
{
	//File name should be "model.txt"
	//containing 30 subject values directly from start of file.

	if(!model_file)
	{
		cout<"\nFile ""model.txt"" not found, Switching to Custom mode\n";
		getCustomModel();
	}

	int temp_file_buffer;
	int fileDefined[genes];
	int j=0;
	int valid_gene_count_flag=0; //only takes first 30 values from file (genes=30)

	while((!model_file.eof())&&(valid_gene_count_flag<30))
	{
		model_file>>temp_file_buffer;

		fileDefined[valid_gene_count_flag]=temp_file_buffer;
		valid_gene_count_flag++;
	}


	model_file.close();

	while(j<=neededOrgs*genes)
	{

		for(int i=1;i<=genes;i++)
	    {   
	    	model[j+i-1]=fileDefined[i-1];
	    	
	    }
	    j+=genes;
	}

}

//Generates random model inlcluding subject value 0 to allele where 0 merely
//indicates that, that free period slot can be assigned to any section.
void getRandomModel()
{
	int randomlyDefined[genes], j=0;

	for(int i=0; i<genes; i++)
	{
		randomlyDefined[i]=rand()%(allele+1);
	}

	while(j<=neededOrgs*genes)
	{

		for(int i=1;i<=genes;i++)
	    {   
	    	model[j+i-1]=randomlyDefined[i-1];
	    	
	    }
	    j+=genes;
	}
}

void getModel() //stress testing
{
	int j=0;
	while(j<=neededOrgs*genes)
	{

		for(int i=1;i<=genes;i+=6)
	    {   
	    	model[j+i-1]=4;
	        model[j+i]=2;
	        model[j+i+1]=1;
	        model[j+i+2]=3;
	        model[j+i+3]=5;
	        model[j+i+4]=6; 	
	    }
	    j+=genes;
	}
}

void getCustomModel()
{
	int userDefined[genes],j=0;

	cout<<"\nEnter the free period model (30 values)\n>>";
	for(int i=0;i<genes;i++)
	{
		cout<<"\n>>";
		cin>>userDefined[i];

	}
	while(j<=neededOrgs*genes)
	{

		for(int i=1;i<=genes;i++)
	    {   
	    	model[j+i-1]=userDefined[i-1];
	    	
	    }
	    j+=genes;
	}
}

void whichModel()
{
	int modelType;

	subjectGuide();
	
	cout<<"\nWhich Free period model to use:\n";
	cout<<"1.Inbuilt\t2.Custom\t3.Random\t4.From file ""model.txt""\n>>";
	cin>>modelType;

	switch(modelType)
	{
		case 1: getModel(); break;
		case 2: getCustomModel(); break;
		case 3: getRandomModel(); break;
		case 4: getModelFromFile(); break;
		default: cout<<"\nWrong model type, enter again: ";
				
	}
}

void showModel(int *m)
{
	for(int i=0;i<genes*neededOrgs;i++)
		cout<<" "<<m[i];
	cout<<endl;
	cout<<endl;
}

void fitnessCalc()
{
	for(int i=0;i<orgs;i++)
	{	
		fitness[i]=0;
		for(int j=0;j<genes*neededOrgs;j++)
		{
			if(model[j]!=curG[i][j])
				fitness[i]+=1;
			if(curG[i][j]==curG[i][j+genes])
				fitness[i]-=1;
			else if(curG[i][j+genes]==curG[i][j+genes+genes])
				fitness[i]-=1;
		}
	}
}

void showFitness()
{
	for(int i=0;i<orgs;i++)
		cout<<" "<<fitness[i];
	cout<<endl;
	cout<<endl;
}

void showPopulation()
{
	for(int i=0;i<orgs;i++)
	{
		for(int j=0;j<genes*neededOrgs;j++)
		{
			cout<<" "<<curG[i][j];
		}
	cout<<endl;
	}
}

void initPopulation()
{
	for(int i=0;i<orgs;i++)
     {  for(int j=0;j<genes*neededOrgs;j++)
        {   
        	curG[i][j]=rand()%allele+1; //the initial population
        }
        
     }
}

int rouletteWheelSelection(int alreadyTakenParent=-1)
{
	int parent=0;
	double chance[orgs];
	int total_fitness;
	double roulette;
	double offset;

	for(int i=0;i<orgs;i++)
		total_fitness+=fitness[i];

	//cout<<endl<<"total_fitness:"<<total_fitness<<endl;

	for(int i=0;i<orgs;i++)
		chance[i]=(double)fitness[i]/total_fitness;


	roulette=(double)rand()/RAND_MAX;
	
	for(int i=0;i<orgs;i++)
	{
		//cout<<"\noffset value:"<<offset<<endl;
		//cout<<"roulette:"<<roulette<<endl;

		offset+=chance[i];

		if((roulette<offset)&&(i!=alreadyTakenParent))
		{
			parent=i;
			break;
		}
	}

	//cout<<"Parent:"<<parent<<endl;
	return parent;

}

int getBestParent(int parentAlreadytaken=-1)
{
	int parent,maxFit=0;;

	for(int i=0;i<orgs;i++)
	{
		if((fitness[i]>=maxFit)&&(i!=parentAlreadytaken))
		{
			maxFit=fitness[i];
			parent=i;
		}
	}
	return parent;
}

int getRandomParent()
{
	int parent=rand()%orgs;
	return parent;
}

//Selection methods 
//1. Random parent
//2. Best parent
//3. Roulette wheel selection
void newGeneration()
{
	int crossoverPoint, mutationProbabilty, parent1, parent2;

	for(int i=0;i<orgs;i++)
	{
		parent1=rouletteWheelSelection(); //getBestParent();
		parent2=rouletteWheelSelection(parent1);//getBestParent(parent1);

		crossoverPoint=rand()%genes;

		for(int j=0;j<genes*neededOrgs;j++)
		{
			mutationProbabilty=rand()%(int)(1.0/mutationRate);

			if(mutationProbabilty==0) //mutate
				nextG[i][j]=rand()%allele+1;
			else if(j<crossoverPoint)
				nextG[i][j]=curG[parent1][j];
			else
				nextG[i][j]=curG[parent2][j];
		}
	}
	curG=nextG;
	fitnessCalc();
	
}

void getElitsm()
{
	for(int i=0;i<orgs;i++)
	{
		if(fitness[i]>maxFit)
		{
			maxFit=fitness[i];
			maxOrganismIndex=i;
			//since cannot convert int**=int* so using maxOrganismIndex to get best org in that gen.
			elitsm=curG; 
		}
	}
}

// ============================================================================
//	Main Function
// ============================================================================

int main()
{	
	srand(time(NULL));
	int i=0;
	memAllocate();
	whichModel();
	showModel(model);
	while(i<generations)
	{
		initPopulation();
		fitnessCalc();
		newGeneration();
		//showFitness();
		cout<<endl<<"["<<i<<" %]"<<"\tFitness:"<<fitness[0]<<endl;
		getElitsm();
		//showPopulation();
		i++;
		//cout<<"\ngen: "<<i;
	}
	cout<<"\nFree period model was:";
	for(int i=0;i<genes*neededOrgs;i++)
		cout<<" "<<model[i];
	cout<<"\nGenerations iterated";
	cout<<"\nResult model was fitness of "<<maxFit;
	cout<<endl;
	for(int i=0;i<orgs;i++)
	{
		for(int j=0;j<genes*neededOrgs;j++)
		{
			if(j%genes==0)
				cout<<endl;
			else
				cout<<" "<<elitsm[i][j];

		}
		cout<<endl;
	}
	cout<<"\nParsing now";
	cout<<endl;
	parse.generateTimeTable(elitsm);
	cout<<endl;
	showModel(model);
	

	cout<<endl;
	cout<<"Press any key to exit";
	cin>>i;
	return 0;

}
//============================================================================