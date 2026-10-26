
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h> 
using namespace std;

/*****************************************************************
 *                     DECLARATIONS                              *
 *****************************************************************/
typedef int NUMBER;
typedef int NAME;
const int  NAMELENG = 20;      /* Maximum length of a name */
const int  MAXNAMES = 100;     /* Maximum number of different names */
const int  MAXINPUT = 5000;     /* Maximum length of an input */
const char*   PROMPT = "-> ";
const char*   PROMPT2 = "> ";
const char  COMMENTCHAR = ';';
const int   TABCODE = 9;        /* in ASCII */

struct EXPLISTREC;
typedef EXPLISTREC* EXPLIST;
enum EXPTYPE {VALEXP,VAREXP,APEXP};
struct EXPREC
{
	  EXPTYPE etype; //what type of expression
	  NUMBER num;
	  NAME varble;
	  NAME optr;
	  EXPLIST args;
};
typedef EXPREC* EXP;

struct EXPLISTREC
{
	  EXP head;
	  EXPLIST tail;
};


struct VALUELISTREC
{
	  NUMBER  head;
	  VALUELISTREC*  tail;
};

typedef VALUELISTREC* VALUELIST;

struct NAMELISTREC
{
	  NAME   head;
	  NAMELISTREC* tail;
};
typedef NAMELISTREC* NAMELIST;

struct  ENVREC
{
	   NAMELIST vars;
	   VALUELIST values;
};

typedef ENVREC* ENV;

struct  FUNDEFREC
{
	   NAME  funname;
	   NAMELIST  formals;
	   EXP  body;
	   FUNDEFREC*  nextfundef;
};
typedef FUNDEFREC* FUNDEF;

FUNDEF  fundefs;

ENV globalEnv;

EXP currentExp;

char userinput[MAXINPUT];
int   inputleng, pos;

char*   printNames[MAXNAMES];
int   numNames, numBuiltins;

int   quittingtime;



/*****************************************************************
 *                     DATA STRUCTURE OP'S                       *
 *****************************************************************/

/* mkVALEXP - return an EXP of type VALEXP with num n            */

EXP mkVALEXP ( NUMBER n)
{
   EXP e;
   e = new EXPREC;
   e->etype = VALEXP;
   e->num = n;
   return e;
}/* mkVALEXP */


/* mkVAREXP - return an EXP of type VAREXP with varble nm  */

EXP mkVAREXP ( NAME nm)
{
   EXP e;
   e = new EXPREC;
   e->etype = VAREXP;
   e->varble = nm;
   return e;
}/* mkVAREXP */


/* mkAPEXP - return EXP of type APEXP w/ optr op and args el     */

EXP mkAPEXP (NAME op, EXPLIST el)
{
   EXP  e;
   e = new EXPREC;
   e->etype = APEXP;
   e->optr = op;
   e->args = el;
   return e;
}/* mkAPEXP */

/* mkExplist - return an EXPLIST with head e and tail el         */

EXPLIST mkExplist (EXP e, EXPLIST el)
{
    EXPLIST newel;
    newel =new EXPLISTREC;
    newel->head = e;
    newel->tail = el;
    return newel;
}/* mkExplist */

/* mkNamelist - return a NAMELIST with head n and tail nl        */

NAMELIST mkNamelist ( NAME nm, NAMELIST nl)
{
   NAMELIST newnl;
   newnl = new NAMELISTREC;
   newnl->head = nm;
   newnl->tail = nl;
   return newnl;
}/* mkNamelist */

/* mkValuelist - return an VALUELIST with head n and tail vl     */

VALUELIST mkValuelist (NUMBER n,  VALUELIST vl)
{
   VALUELIST newvl;
   newvl = new VALUELISTREC;
   newvl->head = n;
   newvl->tail = vl;
   return newvl;
}/* mkValuelist */

/* mkEnv - return an ENV with vars nl and values vl              */

ENV mkEnv ( NAMELIST nl, VALUELIST vl)
{
    ENV rho;
    rho = new ENVREC;
    rho->vars = nl;
    rho->values = vl;
    return rho;
}/* mkEnv */

/* lengthVL - return length of VALUELIST vl      */

int lengthVL ( VALUELIST vl)
{
   int i = 0;
   while (vl != 0)
   {
	 i++;
	 vl = vl->tail;
   }
   return i;
}/* lengthVL */

/* lengthNL - return length of NAMELIST nl    */

int lengthNL ( NAMELIST nl)
{
   int i = 0;
   while( nl !=0 )
   {
	 ++i;
	 nl = nl->tail;
   }
   return i;
}/* lengthNL */

/*****************************************************************
 *                     NAME MANAGEMENT                           *
 *****************************************************************/

/* fetchFun - get function definition of fname from fundefs */

FUNDEF fetchFun ( NAME fname)
{
   FUNDEF  f;
   f = fundefs;
   while (f != 0)
   {
   
	 if (f->funname == fname )
	    return f;
	 f = f->nextfundef;
    }
  return 0;
}/* fetchFun */


/* newFunDef - add new function fname w/ parameters nl, body e   */

void  newFunDef (NAME fname,  NAMELIST nl, EXP e)
{
   FUNDEF f;
   f = fetchFun(fname);
   if (f == 0) /* fname not yet defined as a function */
   {
	 f = new FUNDEFREC;
	 f->nextfundef = fundefs; // place new FUNDEFREC
	 fundefs = f;        // on fundefs list
   }
   f->funname = fname;
   f->formals = nl;
   f->body = e;
}// newFunDef


/* initNames - place all pre-defined names into printNames */

void initNames()
{
   int i =0;
   fundefs = 0;
   printNames[i] = (char* )"loop";    i++;  //0
   printNames[i] = (char* )"if";      i++;  //1
   printNames[i] = (char* )"block";   i++;  //2
   printNames[i] = (char*)"set";      i++;  //3
   printNames[i] = (char* )"+";       i++;  //4
   printNames[i] = (char* )"-";       i++;  //5
   printNames[i] = (char* ) "*";      i++;  //6
   printNames[i] = (char* )"/";       i++;  //7
   printNames[i] = (char* )"=";       i++;  //8
   printNames[i] = (char* )"<";       i++;  //9
   printNames[i] = (char* )">";       i++;  //10
   printNames[i] = (char* )"print";         //11
   numNames = i;
   numBuiltins = i;
}//initNames

/* install - insert new name into printNames  */

NAME install ( char* nm)
{
   int i = 0;
   while (i <= numNames) {
	 if (strcmp( nm,printNames[i] ) == 0)
	    break;
     i++;
   }
   if (i > numNames)
   {
	  numNames = i;
	  printNames[i] = new char[strlen(nm) + 1];
	  strcpy(printNames[i], nm);
   }
   return i;
}// install

/* prName - print name nm              */

void prName ( NAME nm)
{
	 cout<< printNames[nm];
} //prName

/*****************************************************************
 *                        INPUT                                  *
 *****************************************************************/

/* isDelim - check if c is a delimiter   */

int isDelim (char c)
{
   return ( ( c == '(') || ( c == ')') ||( c == ' ')||( c== COMMENTCHAR) );
}

/* skipblanks - return next non-blank position in userinput */

int skipblanks (int p)
{
   while (userinput[p] == ' ')
	++p;
   return p;
}


/* matches - check if string nm matches userinput[s .. s+leng]   */

int matches (int s, int leng,  char* nm)
{
   int i=0;
   while (i < leng )
   {
	 if( userinput[s] != nm[i] )
	    return 0;
	 ++i;
	 ++s;
    }
   if (!isDelim(userinput[s]) )
	  return 0;
   return 1;
}/* matches */



/* nextchar - read next char - filter tabs and comments */

void nextchar (char& c)
{
    scanf("%c", &c);
    if (c == COMMENTCHAR )
    {
	  while ( c != '\n' )
		scanf("%c",&c);
    }
}


/* readParens - read char's, ignoring newlines, to matching ')' */
void readParens()
{
   int parencnt; /* current depth of parentheses */
   char c;
   parencnt = 1; // '(' just read
   do
   {
	  if (c == '\n')
	    cout <<PROMPT2;
	  cout.flush();
	  nextchar(c);
	  pos++;
	  if (pos == MAXINPUT )
	  {
		cout <<"User input too long\n";
		exit(1);
	  }
	  if (c == '\n' )
		userinput[pos] = ' ';
	  else
		userinput[pos] = c;
	  if (c == '(')
		++parencnt;
	  if (c == ')')
	    parencnt--;
	}
    while (parencnt != 0 );
} //readParens

/* readInput - read char's into userinput */

void readInput()
{
    char  c;
    cout << PROMPT;
    cout.flush();
    pos = 0;
    do
	 {
	    ++pos ;
	    if (pos == MAXINPUT )
	    {
		    cout << "User input too long\n";
		    exit(1);
	    }
	    nextchar(c);
	    if (c == '\n' )
		   userinput[pos] = ' ';
	    else
		   userinput[pos] = c;
	    if (userinput[pos] == '(' )
		  readParens();
	 }
	while (c != '\n');
	inputleng = pos;
	userinput[pos+1] = COMMENTCHAR; // sentinel
}


/* reader - read char's into userinput; be sure input not blank  */

void reader ()
{
    do
    {
	  readInput();
	  pos = skipblanks(1);
    }
    while( pos > inputleng); // ignore blank lines
}

/* parseName - return (installed) NAME starting at userinput[pos]*/

NAME parseName()
{
   char nm[20]; // array to accumulate characters
   int leng; // length of name
   leng = 0;
   while ( (pos <= inputleng) && !isDelim(userinput[pos]) )
   {
	    
	    nm[leng] = userinput[pos];
	    ++pos;
	    ++leng;
   }
   if (leng == 0)
   {
	   cout<<"Error: expected name, instead read: "<< userinput[pos]<<endl;
	   exit(1);
   }
   nm[leng] = '\0';
   pos = skipblanks(pos); // skip blanks after name
  
   return ( install(nm) );
}// parseName

/* isDigits - check if sequence of digits begins at pos   */

int isDigits (int pos)
{
   if ( ( userinput[pos] < '0' ) ||  ( userinput[pos] > '9' ) )
	  return 0;
   while ( ( userinput[pos] >='0') && ( userinput[pos] <= '9') )
	 ++pos;
   if (!isDelim(userinput[pos] ))
	 return 0;
   return 1;
}// isDigits


/* isNumber - check if a number begins at pos  */

int isNumber (int pos)
{
   return ( isDigits(pos) || 
             ( (userinput[pos] == '-') && isDigits(pos+1))
            ||( (userinput[pos] == '+') && isDigits(pos+1)));
}// isNumber

/* parseVal - return number starting at userinput[pos]   */

NUMBER parseVal()
{
   int  n=0, sign=1;
   if (userinput[pos] == '+')
      ++pos;
   if (userinput[pos] == '-')
   {
	 sign = -1;
	 ++pos;
   }
   while ( (userinput[pos] >= '0') && ( userinput[pos] <= '9') )
   {
	    n = 10*n + userinput[pos] -'0';
	    ++pos;
   }
   pos = skipblanks(pos); // skip blanks after number
   return ( (NUMBER) n*sign);
}// parseVal

EXPLIST parseEL();

/* parseExp - return EXP starting at userinput[pos]  */

EXP parseExp()
{
   NAME nm;
   EXPLIST el;
   pos = skipblanks(pos);
   
   if ( userinput[pos] == '(' )
   {// APEXP
	  pos = skipblanks(pos+1); // skip '( ..'
	  nm = parseName();
	  el = parseEL();
	  return ( mkAPEXP(nm, el));
   }
   if (isNumber(pos))
	  return ( mkVALEXP((NUMBER)parseVal() ));  // VALEXP
   return ( mkVAREXP((NAME)parseName() ) ); // VAREXP
}// parseExp

/* parseEL - return EXPLIST starting at userinput[pos]  */

EXPLIST parseEL()
{
   EXP e;
   EXPLIST el;
   if ( userinput[pos] == ')')
   {
	 pos = skipblanks(pos+1); // skip ') ..'
	 return 0;
   }
   e = parseExp();
   el = parseEL();
   return ( mkExplist(e, el));
}// parseEL


/* parseNL - return NAMELIST starting at userinput[pos]  */

NAMELIST parseNL()
{
    NAME nm;
    NAMELIST nl;
   if ( userinput[pos] == ')' )
   {
	  pos = skipblanks(pos+1); // skip ') ..'
	  return 0;
   }
   nm = parseName();
   nl = parseNL();
   return ( mkNamelist(nm, nl));
}// parseNL

/* parseDef - parse function definition at userinput[pos]   */

NAME parseDef()
{
    NAME fname;        // function name
    NAMELIST nl;       // formal parameters
    EXP e;             // body
    pos = skipblanks(pos+1); // skip '( ..'
    pos = skipblanks(pos+6); // skip 'define ..'
    fname = parseName();
    pos = skipblanks(pos+1); // skip '( ..'
    nl = parseNL();
    e = parseExp();
    pos = skipblanks(pos+1); // skip ') ..'
    newFunDef(fname, nl, e);
   return ( fname);
}// parseDef

/*****************************************************************
 *                     ENVIRONMENTS                              *
 *****************************************************************/

/* emptyEnv - return an environment with no bindings */

ENV emptyEnv()
{
   return  mkEnv(0, 0);
}

/* bindVar - bind variable nm to value n in environment rho */

void bindVar ( NAME nm,  NUMBER n,  ENV rho)
{
   rho->vars = mkNamelist(nm, rho->vars); 
   rho->values = mkValuelist(n, rho->values); 
}

/* findVar - look up nm in rho   */

VALUELIST findVar ( NAME nm, ENV rho)
{
   NAMELIST  nl = rho->vars;
   VALUELIST  vl = rho->values;
   while (nl != 0) { // reminder that 0 is our stand-in for the null pointer
      if (nl->head == nm) return vl; 
      nl = nl->tail;
      vl = vl->tail; 
   }
   return 0;
}


/* assign - assign value n to variable nm in rho   */

void  assign (NAME nm,  NUMBER n, ENV rho)
{
   NAMELIST nl = rho->vars;
   VALUELIST vl = rho->values;
   while (nl != 0) {
      if (nl->head == nm) {
         vl->head = n;
         return;
      } 
      nl = nl->tail;
      vl = vl->tail; 
   }
   return;
}// assign

/* fetch - return number bound to nm in rho */

NUMBER fetch ( NAME nm, ENV rho)
{
   NAMELIST nl = rho->vars;
   VALUELIST  vl = rho->values;
   while (nl != 0){
      if (nl->head == nm){
         return (vl->head); 
      }
      nl = nl->tail;
      vl = vl->tail;
   }
   cout << "Unbound variable: ";
   prName(nm);
   cout << endl;
   exit(1);
}

/* isBound - check if nm is bound in rho  */

int isBound ( NAME nm, ENV rho)
{
   return findVar(nm, rho) != 0;
}


/*****************************************************************
 *                     NUMBERS                                   *
 *****************************************************************/

/* prValue - print number n    */

void prValue ( NUMBER n )
{
   cout << n;
}
// prValue


/* isTrueVal - return true if n is a true (non-zero) value  */

int isTrueVal ( NUMBER n)
{
   return ( n != 0 );
}// isTrueVal


/* arity - return number of arguments expected by op  */

int arity ( int op)
{

	if (op == 11) //print
	   return  1;
	return 2;
}// arity

/* applyValueOp - apply VALUEOP op to arguments in VALUELIST vl */

NUMBER applyValueOp ( int op,  VALUELIST vl)
{
   NUMBER  n, n1, n2;
   if ( arity(op) != lengthVL(vl) )
   {
	  cout <<	"Wrong number of arguments to  ";
	  prName(op);
	  cout <<endl;
   }
   n1 = vl->head;
   if (op != 11) // every operator except print needs a second value
      n2 = vl->tail->head; 
   switch (op) {
      case 4:   // +
         n = n1 + n2;
         break;

      case 5:   // -
         n = n1 - n2;
         break;

      case 6:   // *
         n = n1 * n2;
         break;

      case 7:   // /
         n = n1 / n2;
         break;

      case 8:   // =
         n = (n1 == n2);
         break;

      case 9:   // <
         n = (n1 < n2);
         break;

      case 10:  // >
         n = (n1 > n2);
         break;

      case 11:  // print
         prValue(n1);
         n = n1;
         break;
   }
   return n;
}// applyValueOp


/*****************************************************************
 *                     EVALUATION                                *
 *****************************************************************/

/* eval -- evaluate an expression. */
NUMBER eval ( EXP e,  ENV rho){
   switch (e->etype) {
      case VALEXP:
         return e->num;
      case VAREXP:
         return fetch(e->varble, rho); 
      case APEXP:
         if (e->optr > numBuiltins) return applyUserFun(e->optr, evalList(e->args, rho)); 
         if (e->optr < 4) return applyCtrlOp(e->optr, e->args, rho); 
         return applyValueOp(e->optr, evalList(e->args, rho));
   }
   return 0;
}

VALUELIST evalList ( EXPLIST el, ENV rho)
{
    NUMBER h;
    VALUELIST t;
   //do this
}// evalList

/* applyUserFun - look up definition of nm and apply to actuals  */

NUMBER applyUserFun ( NAME nm, VALUELIST actuals)
{
    FUNDEF f;
    ENV rho;
  //do this
}// applyUserFun

/* applyCtrlOp - apply CONTROLOP op to args in rho   */

NUMBER applyCtrlOp ( int op,  EXPLIST args, ENV rho )
{
   NUMBER  n =0;
   if (op == 0) {
   	//do this
   }
	

   if ( op == 1)
   {
	 //do this
    };
    if( op == 3)
    {
	   //do this
    }
    if( op == 2)
    {
	 //do this
    }
    return n;
}// applyCtrlOp

// eval - return value of expression e in local environment rho

NUMBER eval ( EXP e,  ENV rho)
{
   switch (e->etype)
   {
	  case VALEXP: return (e->num);
	  case VAREXP:  //do this
				 
	  case APEXP:  if (e->optr > numBuiltins)
				      return  applyUserFun(e->optr, evalList(e->args, rho));
			       else {
				          if ( e->optr< 4 )
				    	     return applyCtrlOp(e->optr, e->args, rho);
				           return applyValueOp(e->optr, evalList(e->args, rho));
				        }
    }
    return 0;
} // eval

/*****************************************************************
 *                     READ-EVAL-PRINT LOOP                      *
 *****************************************************************/

int main()
{
   
   initNames();
   globalEnv = emptyEnv();

   quittingtime = 0;
   while (!quittingtime)
   {
   	
	 reader();
	 if ( matches(pos, 4, (char* )"quit"))
	    quittingtime = 1;
	 else if( (userinput[pos] == '(') &&
		    matches(skipblanks(pos+1), 6, (char* )"define")  )
	 {
		    prName(parseDef());
		    cout <<endl;
	 }
	 else {
			currentExp = parseExp();
			prValue(eval(currentExp, emptyEnv() ));
			cout <<endl<<endl;
		 }
	}// while
    return 0;
}


