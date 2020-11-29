/* Steven Andrews, started 10/22/2001.
 This is the header for data structures for the Smoldyn program.
 See documentation called SmoldynUsersManual.pdf and SmoldynCodeDoc.pdf, and the Smoldyn
 website, which is at www.smoldyn.org.
 Copyright 2003-2016 by Steven Andrews.  This work is distributed under the terms
 of the Gnu Lesser General Public License (LGPL). */

#ifndef __smoldyn_h__
#define __smoldyn_h__

#include <ctime>
#include <cstdio>
#include <array>
using namespace std;
#include "smoldynconfigure.h"			// generated by CMake from smoldynconfigure.h.in

#ifdef ENABLE_PYTHON_CALLBACK
#define MAX_PY_CALLBACK 10
#include "../python/CallbackFunc.h"
#endif

//#ifdef OPTION_NSV
//  #include "nsvc.h"
//#endif


#ifdef WINDOWS_BUILD
  #define LLUFORMAT "%I64u"				// MinGW doesn't use standard printf formatting
#else
  #define LLUFORMAT "%llu"
#endif


#ifdef OPTION_VTK
  #include "vtkwrapper.h"
#endif

#ifdef OPTION_VCELL
	#include <string>
	typedef struct VolumeSamples * VolumeSamplesPtr;
	class AbstractMesh;
	class ValueProvider;
	class ValueProviderFactory;
	using std::string;
#endif

/****************************************************************************/
/********************************** General *********************************/
/****************************************************************************/

#define DIMMAX 3							// maximum system dimensionality
#define VERYCLOSE 1.0e-12			// distance that's safe from round-off error
#define VERYLARGE 1.0e+20			// distance that's large but safe from overflow error

enum StructCond {SCinit,SClists,SCparams,SCok};


/****************************************************************************/
/*************** Data structures for included libraries *********************/
/****************************************************************************/

/*** string2.h ***/

#ifndef __string2_h
#define __string2_h

#define STRCHAR 256
#define STRCHARLONG 4000

#endif

/*** queue.h ***/

#ifndef __queue_h
#define __queue_h

#include <limits.h>
#if !defined(LLONG_MAX) || defined(WIN32)
	typedef long int Q_LONGLONG;
	#define Q_LLONG_MAX LONG_MAX
	#define Q_LLI "%li"
#else
	typedef long long int Q_LONGLONG;
	#define Q_LLONG_MAX LLONG_MAX
	#define Q_LLI "%lli"
#endif

enum Q_types {Qusort,Qvoid,Qint,Qdouble,Qlong};

typedef struct qstruct{
	enum Q_types type;
	void **kv;
	int *ki;
	double *kd;
	Q_LONGLONG *kl;
	int (*keycmp)(void *,void *);
	void **x;
	int n;
	int f;
	int b; } *queue;

#endif

/***  List.h ***/

#ifndef __list_h
#define __list_h

typedef struct liststructli{
	int max;
	int n;
	long int *xs;
	} *listptrli;

typedef struct liststructv{
	int max;
	int n;
	void **xs;
	} *listptrv;

#endif

/*** SimCommand.h ***/

#ifndef __SimCommand_h__
#define __SimCommand_h__

enum CMDcode {CMDok,CMDwarn,CMDpause,CMDstop,CMDabort,CMDnone,CMDcontrol,CMDobserve,CMDmanipulate,CMDctrlORobs,CMDall};

typedef struct cmdstruct {
	struct cmdsuperstruct *cmds;	// owning command superstructure
	double on;						// first command run time
	double off;						// last command run time
	double dt;						// time interval between commands
	double xt;						// multiplicative time interval
	Q_LONGLONG oni;				// first command run iteration
	Q_LONGLONG offi;			// last command run iteration
	Q_LONGLONG dti;				// iterations between commands
	Q_LONGLONG invoke;		// number of times command has run
	char *str;						// command string
	char *erstr;					// storage space for error string
	int i1,i2,i3;					// integers for generic use
	double f1,f2,f3;			// doubles for generic use
	void *v1,*v2,*v3;			// pointers for generic use
	void (*freefn)(struct cmdstruct*);	// free command memory
	} *cmdptr;

typedef struct cmdsuperstruct {
	queue cmd;						// queue of normal run-time commands
	queue cmdi;						// queue of integer time commands
	enum CMDcode (*cmdfn)(void*,cmdptr,char*);	// function that runs commands
	void *cmdfnarg;				// function argument (e.g. sim)
	int iter;							// number of times integer commands have run
	int maxfile;					// number of files allocated
	int nfile;						// number of output files
	char root[STRCHAR];		// file path
	char froot[STRCHAR];	// more file path, used after root
	char **fname;					// file name [fid]
	int *fsuffix;					// file suffix [fid]
	int *fappend;					// 0 for overwrite, 1 for append [fid]
	FILE **fptr;					// file pointers [fid]
	double flag;					// global command structure flag
	int precision;				// precision for output commands
	char outformat;				// output format, 's' or 'c'
	} *cmdssptr;

#endif

/*** nsvc.h ***/

#ifndef NSVC_H_
#define NSVC_H_

#ifdef NSVC_CPP
	#include <string>
	#include "Kairos.h"
	using Kairos::NextSubvolumeMethod;

	#if defined(HAVE_VTK)
		#include <vtkUnstructuredGrid.h>
	#else
		typedef struct vtkUnstructuredGrid vtkUnstructuredGrid;
	#endif
#else
	typedef struct vtkUnstructuredGrid vtkUnstructuredGrid;
	typedef struct NextSubvolumeMethod NextSubvolumeMethod;
#endif

#endif

/********************************* Molecules ********************************/

#define MSMAX 5
#define MSMAX1 6
enum MolecState {MSsoln,MSfront,MSback,MSup,MSdown,MSbsoln,MSall,MSnone,MSsome};
enum MolListType {MLTsystem,MLTport,MLTnone};
#define PDMAX 6
enum PatternData {PDalloc,PDnresults,PDnspecies,PDmatch,PDsubst,PDrule};

typedef struct moleculestruct {
	unsigned long long serno;		// serial number
	int list;										// destination list number (ll)
	double *pos;								// dim dimensional vector for position [d]
	double *posx;								// dim dimensional vector for old position [d]
	double *via;								// location of last surface interaction [d]
	double *posoffset;					// position offset arising from jumps [d]
	int ident;									// species of molecule; 0 is empty (i)
	enum MolecState mstate;			// physical state of molecule (ms)
	struct boxstruct *box;			// pointer to box which molecule is in
	struct panelstruct *pnl;		// panel that molecule is bound to if any
	struct panelstruct *pnlx;		// old panel that molecule was bound to if any
	} *moleculeptr;

typedef struct molsuperstruct {
	enum StructCond condition;  // structure condition
	struct simstruct *sim;      // simulation structure
	int maxspecies;             // maximum number of species
	int nspecies;               // number of species, including empty mols.
	char **spname;              // names of molecular species [i]
	int maxpattern;             // maximum number of patterns
	int npattern;               // actual number of patterns
	char **patlist;             // list of patterns [pat]
	int **patindex;             // species indices for patterns [pat][j]
	char **patrname;            // pattern reaction name if any [pat]
	double **difc;              // diffusion constants [i][ms]
	double **difstep;           // rms diffusion step [i][ms]
	double ***difm;             // diffusion matrix [i][ms][d]
	double ***drift;            // drift vector [i][ms][d]
	double *****surfdrift;      // surface drift [i][ms][s][ps][d]
	double **display;           // display size of molecule [i][ms]
	double ***color;            // RGB color vector [i][ms]
	int **exist;                // flag for if molecule could exist [i][ms]
	moleculeptr *dead;          // list of dead molecules [m]
	int maxdlimit;              // maximum allowed size of dead list
	int maxd;                   // size of dead molecule list
	int nd;                     // total number of molecules in dead list
	int topd;                   // index for dead list; above are resurrected
	int maxlist;                // allocated number of live lists
	int nlist;                  // number of live lists
	int **listlookup;           // lookup table for live lists [i][ms]
	char **listname;            // names of molecule lists [ll]
	enum MolListType *listtype; // types of molecule lists [ll]
	moleculeptr **live;         // live molecule lists [ll][m]
	int *maxl;                  // size of molecule lists [ll]
	int *nl;                    // number of molecules in live lists [ll]
	int *topl;                  // live list index; above are reborn [ll]
	int *sortl;                 // live list index; above need sorting [ll]
	int *diffuselist;           // 1 if any listed molecs diffuse [ll]
	unsigned long serno;        // serial number for next resurrected molec.
	int ngausstbl;              // number of elements in gausstbl
	double *gausstbl;           // random numbers for diffusion
	int *expand;                // expansion with rule-based modeling [i]
	long int touch;             // counter for molecule modification
	} *molssptr;

/*********************************** Walls **********************************/

typedef struct wallstruct {
	int wdim;										// dimension number of perpendicular to wall
	int side;										// low side of space (0) or high side (1)
	double pos;									// position of wall along dim axis
	char type;									// properties of wall
	struct wallstruct *opp; 		// pointer to opposite wall
	} *wallptr;

/********************************* Reactions ********************************/

#define MAXORDER 3

// NOTE:
// Increasing MAXPRODUCT value to 256 
// see https://github.com/ssandrews/Smoldyn/issues/18 for details.
// #define MAXPRODUCT 256
constexpr size_t MAXPRODUCT=256;

enum RevParam {RPnone,RPirrev,RPconfspread,RPbounce,RPpgem,RPpgemmax,RPpgemmaxw,RPratio,RPunbindrad,RPpgem2,RPpgemmax2,RPratio2,RPoffset,RPfixed};

enum SpeciesRepresentation {SRparticle,SRlattice,SRboth,SRnone,SRfree};

#ifdef OPTION_VCELL
class ValueProvider;
typedef ValueProvider* valueproviderptr;
#endif

typedef struct rxnstruct {
	struct rxnsuperstruct *rxnss;	// pointer to superstructure
	char *rname;								// pointer to name of reaction
	int *rctident;							// list of reactant identities [rct]
	enum MolecState *rctstate;	// list of reactant states [rct]
	int *permit;								// permissions for reactant states [ms]
	int nprod;									// number of products
	int *prdident;							// list of product identities [prd]
	enum MolecState *prdstate;	// list of product states [prd]
	enum SpeciesRepresentation *rctrep;	// reactant particle/lattice [rct]
	enum SpeciesRepresentation *prdrep;	// product particle/lattice [prd]
	long int *prdserno;					// list of product serno rules [prd]
	int *prdintersurf;					// list of product intersurface rules [prd]
	listptrli logserno;					// list of serial nums for logging reaction
	char *logfile;							// filename for logging reaction
#ifdef OPTION_VCELL
	valueproviderptr rateValueProvider;			// requested reaction rate
#endif
	double rate;								// requested reaction rate
	int multiplicity;						// rate multiplier
	double bindrad2;						// squared binding radius, if appropriate
	double prob;								// reaction probability
	double chi;									// diffusion-limited fraction
	double tau;									// characteristic reaction time
	enum RevParam rparamt;			// type of parameter in rpar
	double rparam;							// parameter for reaction of products
	double unbindrad;						// unbinding radius, if appropriate
	double **prdpos;						// product position vectors [prd][d]
	int disable;								// 1 if reaction is disabled
	struct compartstruct *cmpt;	// compartment reaction occurs in, or NULL
	struct surfacestruct *srf;	// surface reaction on, or NULL
	} *rxnptr;

typedef struct rxnsuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	int order;									// order of reactions listed: 0, 1, or 2
	int maxspecies;							// maximum number of species
	int maxlist;								// copy of maximum number of molecule lists
	int *nrxn;									// number of rxns for each reactant set [i]
	int **table;								// lookup table for reaction numbers [i][j]
	int maxrxn;									// allocated number of reactions
	int totrxn;									// total number of reactions listed
	char **rname;								// names of reactions [r]
	rxnptr *rxn;								// list of reactions [r]
	int *rxnmollist;						// live lists that have reactions [ll]
	} *rxnssptr;


/********************************** Rules ***********************************/

enum RuleType {RTreaction,RTdifc,RTdifm,RTdrift,RTsurfdrift,RTmollist,RTdispsize,RTcolor,RTsurfaction,RTsurfrate,RTsurfrateint,RTnone};

typedef struct rulestruct {
	struct rulesuperstruct *ruless;	// pointer to superstructure
	enum RuleType ruletype;			// type of rule
	char *rulename;							// pointer to name of rule
	char *rulepattern;					// pattern for the rule
	int rulenresults;								// number of results in index used by rule
	int *ruledetailsi;					// list of species states and restrictions
	double *ruledetailsf;				// floating point details
	double rulerate;						// rate constant
	rxnptr rulerxn;							// template reaction for reaction rules
	} *ruleptr;


typedef struct rulesuperstruct {
	struct simstruct *sim;			// simulation structure
	int maxrule;								// allocated size of rule list
	int nrule;									// actual size of rule list
	char **rulename;						// list of rule names
	ruleptr *rule;							// list of rules
	int ruleonthefly;						// for expanding rules on the fly
	} *rulessptr;


/********************************* Surfaces *********************************/

constexpr int PSMAX=6; // maximum number of panel shapes
enum PanelFace {PFfront,PFback,PFnone,PFboth};
enum PanelShape {PSrect,PStri,PSsph,PScyl,PShemi,PSdisk,PSall,PSnone};

// NOTE: It is used in libsmoldyn.cpp
constexpr std::array<PanelShape, 6> AllPanels_arr{PanelShape::PSrect, PanelShape::PStri,
    PanelShape::PSsph, PanelShape::PScyl, PanelShape::PShemi, PanelShape::PSdisk};


enum SrfAction {SAreflect,SAtrans,SAabsorb,SAjump,SAport,SAmult,SAno,SAnone,SAadsorb,SArevdes,SAirrevdes,SAflip};
enum DrawMode {DMno=0,DMvert=1,DMedge=2,DMve=3,DMface=4,DMvf=5,DMef=6,DMvef=7,DMnone};
enum SMLflag {SMLno=0,SMLdiffuse=1,SMLreact=2,SMLsrfbound=4};

typedef struct surfactionstruct {
	int *srfnewspec;						// surface convert mol. species [ms]
	double *srfrate;						// surface action rate [ms]
#ifdef OPTION_VCELL
	valueproviderptr* srfRateValueProvider;	//rate for surface actions: asorption, desorption, transmission...etc.
#endif
	double *srfprob;						// surface action probability [ms]
	double *srfcumprob;					// surface cumulative probability [ms]
	int *srfdatasrc;						// surface data source [ms]
	double *srfrevprob;					// probability of reverse action [ms]
	} *surfactionptr;

typedef struct panelstruct {
	char *pname;								// panel name (reference, not owned)
	enum PanelShape ps;					// panel shape
	struct surfacestruct *srf;	// surface that owns this panel
	int npts;										// number of defining points
	double **point;							// defining points, [number][d]
	double **oldpoint;					// prior defining points, [number][d]
	double front[DIMMAX];				// front parameters, which depend on the shape
	double oldfront[DIMMAX];		// prior front parameters [d]
	struct panelstruct *jumpp[2];// panel to jump to, if appropriate [face]
	enum PanelFace jumpf[2];		// face to jump to, if appropriate [face]
	int maxneigh;								// maximum number of neighbor panels
	int nneigh;									// number of neighbor panels
	struct panelstruct **neigh;	// list of neighbor panels [p]
	double *emitterabsorb[2];		// absorption for emitters [face][i]
	} *panelptr;

typedef struct surfacestruct {
	char *sname;								// surface name (reference, not owned)
	struct surfacesuperstruct *srfss;	// owning surface superstructure
	int selfindex;							// index of self
	enum SrfAction ***action;		// action for molecules [i][ms][face]
	surfactionptr ***actdetails;	// action details [i][ms][face]
	int neighhop;								// whether molecules hop between neighbors
	double fcolor[4];						// RGBA color vector for front
	double bcolor[4];						// RGBA color vector for back
	double edgepts;							// thickness of edge for drawing
	unsigned int edgestipple[2];	// edge stippling [factor,pattern]
	enum DrawMode fdrawmode;		// polygon drawing mode for front
	enum DrawMode bdrawmode;		// polygon drawing mode for back
	double fshiny;							// front shininess
	double bshiny;							// back shininess
	int maxpanel[PSMAX];				// allocated number of panels [ps]
	int npanel[PSMAX];					// actual number of panels [ps]
	char **pname[PSMAX];				// names of panels [ps][p]
	panelptr *panels[PSMAX];		// list of panels [ps][p]
	struct portstruct *port[2];	// port, if any, for each face [face]
	double totarea;							// total surface area
	int totpanel;								// total number of panels
	double *areatable;					// cumulative panel areas [pindex]
	panelptr *paneltable;				// sequential list of panels [pindex]
	int *maxemitter[2];					// maximum number of emitters [face][i]
	int *nemitter[2];						// number of emitters [face][i]
	double **emitteramount[2];	// emitter amounts [face][i][emit]
	double ***emitterpos[2];		// emitter positions [face][i][emit][d]
	int nmollist;								// number of molecule live lists
	int *maxmol;								// allocated size of live lists [ll]
	int *nmol;									// number of molecules in live lists [ll]
	moleculeptr **mol;					// live molecules on the surface [ll][m]
	 } *surfaceptr;

typedef struct surfacesuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	int maxspecies;							// maximum number of molecular species
	int maxsrf;									// maximum number of surfaces
	int nsrf;										// number of surfaces
	double epsilon;							// max deviation of surface-point from surface
	double margin;							// panel margin away from edge
	double neighdist;						// neighbor distance value
	char **snames;							// surface names [s]
	surfaceptr *srflist;				// list of surfaces [s]
	int maxmollist;							// number of molecule lists allocated
	int nmollist;								// number of molecule lists used
	enum SMLflag *srfmollist;		// flags for molecule lists to check [ll]
	} *surfacessptr;

/*********************************** Boxes **********************************/

typedef struct boxstruct {
	int *indx;									// dim dimensional index of the box [d]
	int nneigh;									// number of neighbors in list
	int midneigh;								// logical middle of neighbor list
	struct boxstruct **neigh;		// all box neighbors, using sim. accuracy
	int *wpneigh;								// wrapping code of neighbors in list
	int nwall;									// number of walls in box
	wallptr *wlist;							// list of walls that cross the box
	int maxpanel;								// allocated number of panels in box
	int npanel;									// number of surface panels in box
	panelptr *panel;						// list of panels in box
	int *maxmol;								// allocated size of live lists [ll]
	int *nmol;									// number of molecules in live lists [ll]
	moleculeptr **mol;					// lists of live molecules in the box [ll][m]
	} *boxptr;

typedef struct boxsuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	int nlist;									// copy of number of molecule lists
	double mpbox;								// requested number of molecules per box
	double boxsize;							// requested box width
	double boxvol;							// actual box volumes
	int nbox;										// total number of boxes
	int *side;									// number of boxes on each side of space
	double *min;								// position vector for low corner of space
	double *size;								// length of each side of a box
	boxptr *blist; 							// actual array of boxes
	} *boxssptr;

/******************************* Compartments *******************************/

enum CmptLogic {CLequal,CLequalnot,CLand,CLor,CLxor,CLandnot,CLornot,CLnone};

typedef struct compartstruct {
	struct compartsuperstruct *cmptss;	// compartment superstructure
	char *cname;								// compart. name (reference, not owned)
	int selfindex;							// index of self	//?? needs documenting
	int nsrf;										// number of bounding surfaces
	surfaceptr *surflist;				// list of bounding surfaces [s]
	int npts;										// number of inside-defining points
	double **points;						// list of inside-defining points [k][d]
	int ncmptl;									// number of logic compartments
	struct compartstruct **cmptl;	// list of logic compartments [cl]
	enum CmptLogic *clsym;			// compartment logic symbol [cl]
	double volume;							// volume of compartment
	int maxbox;									// maximum number of boxes in compartment
	int nbox;										// number of boxes inside compartment
	boxptr *boxlist;						// list of boxes inside compartment [b]
	double *boxfrac;						// fraction of box volume that's inside [b]
	double *cumboxvol;					// cumulative cmpt. volume of boxes [b]
	} *compartptr;

typedef struct compartsuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	int maxcmpt;								// maximum number of compartments
	int ncmpt;									// actual number of compartments
	char **cnames;							// compartment names [c]
	compartptr *cmptlist;				// list of compartments [c]
	} *compartssptr;

/*********************************** Ports **********************************/

typedef struct portstruct {
	struct portsuperstruct *portss;	// port superstructure
	char *portname;							// port name (reference, not owned)
	surfaceptr srf;							// porting surface (ref.)
	enum PanelFace face;				// active face of porting surface
	int llport;									// live list number for buffer
	} *portptr;

typedef struct portsuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	int maxport;								// maximum number of ports
	int nport;									// actual number of ports
	char **portnames;						// port names
	portptr *portlist;					// list of ports
	} *portssptr;


/*********************************** lattice **********************************/

enum LatticeType {LATTICEnone,LATTICEnsv,LATTICEpde};

typedef struct latticestruct {
  struct latticesuperstruct *latticess;	// lattice superstructure
  char *latticename;          // lattice name (reference, not owned)
  enum LatticeType type;      // type of lattice
  double min[DIMMAX];         // lower spatial boundaries
  double max[DIMMAX];         // upper spatial boundaries
  double dx[DIMMAX];          // lattice lengthscale (subvolume width)
  char btype[DIMMAX];         // boundary type (r)eflective or (p)eriodic
  portptr port;               // interface port (ref.)
	int **convert;							// convert to particle at port, 0 or 1 [lat.species][face] ??
  int maxreactions;           // maximum number of reactions
  int nreactions;             // number of reactions
  rxnptr *reactionlist;       // list of reactions
  int *reactionmove;          // 0 or 1 for moving reactions
  int maxsurfaces;           // maximum number of surface
  int nsurfaces;             // number of surface
  surfaceptr *surfacelist;       // list of surface
  int maxspecies;             // maximum number of species
  int nspecies;               // number of species
  int *species_index;					// species indices
  int *maxmols;               // allocated size of molecule list [lat.species]
  int *nmols;                 // number of individual molecules [lat.species]
  double*** mol_positions;    // molecule positions [lat.species][nmols][dim]
#ifdef OPTION_NSV
  NextSubvolumeMethod* nsv;		// nsv class
  NextSubvolumeMethod* pde;		// pde class
#else
  void *nsv;
  void *pde;
#endif
	} *latticeptr;

typedef struct latticesuperstruct {
  enum StructCond condition;	// structure condition
  struct simstruct *sim;			// simulation structure
  int maxlattice;							// maximum number of lattices
  int nlattice;								// actual number of lattices
  char **latticenames;				// lattice names
  latticeptr *latticelist;		// list of lattices
	} *latticessptr;

/********************************* Filaments ********************************/

typedef struct filamentstruct {
	struct filamentsuperstruct *filss;	// filament superstructure
	char *fname;								// filament name
	double color[4];						// filament color
	double edgepts;							// thickness of edge for drawing
	unsigned int edgestipple[2];	// edge stippling [factor, pattern]
	enum DrawMode drawmode;			// polygon drawing mode
	double shiny;								// shininess
	int maxseg;									// number of segments allocated
	int nseg;										// number of segments
	int front;									// front index
	int back;										// back index
	double **sxyz;							// Coords. for segment ends [seg][3]
	double *slen;								// segment length [seg]
	double **sypr;							// relative ypr angles [seg][3]
	double **sdcm;							// relative dcm [seg][9]
	double **sadcm;							// absolute segment orientation [seg][9]
	double *sthk;								// thickness of segment [nmax], [0,inf)
	double stdlen;								// minimum energy segment length
	double stdypr[3];						// minimum energy bend angle
	double klen;								// force constant for length
	double kypr[3];							// force constant for angle
	double kT;									// thermodynamic temperature, [0,inf)
	double treadrate;						// treadmilling rate constant
	} *filamentptr;

typedef struct filamentsuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	int maxfil;									// maximum number of filaments
	int nfil;										// actual number of filaments
	char **fnames;							// filament names
	filamentptr *fillist;				// list of filaments
	} *filamentssptr;


/******************************** BioNetGen *********************************/

typedef struct bngstruct {
	struct bngsuperstruct *bngss; // bng superstructure
	char *bngname;              // bng name
  int bngindex;               // index of this bng structure
	double unirate;							// multiplier for unimolecular rxn rates
	double birate;							// multiplier for bimolecular rxn rates
  
	int maxparams;              // maximum number of numeric parameters
	int nparams;                // actual number of numeric parameters
	char **paramnames;          // names of parameters [index]
  char **paramstrings;        // strings for parameter values [index]
	double *paramvalues;        // actual parameter values [index]

  int maxmonomer;             // maximum number of monomers
  int nmonomer;               // actual number of monomers
  char **monomernames;        // names of monomers [index]
  int *monomercount;          // monomer count work space [index]
	double *monomerdifc;				// diffusion coefficient of monomer [index]
	double *monomerdisplaysize;	// display size of monomer [index]
	double **monomercolor;			// color of monomer [index][RGB]
	enum MolecState *monomerstate; // default monomer state [index]
	int bngmaxsurface;					// local copy of nsurface
	enum SrfAction ***monomeraction;	// monomer surface actions [index][srf][face]
	surfactionptr ***monomeractdetails;	// monomer action details [index][srf][face]

	int maxbspecies;            // maximum number of bng species
	int nbspecies;              // actual number of bng species
	char **bsplongnames;        // complete bng species names [index]
  char **bspshortnames;       // shortened bng species names [index]
	enum MolecState *bspstate;	// default species state [index]
	char **bspcountstr;         // strings for initial bng species counts [index]
  double *bspcount;           // actual initial bng species counts [index]
  int *spindex;               // smoldyn index of this species [index]

	int maxbrxns;               // maximum number of bng reactions
	int nbrxns;                 // acutal number of bng reactions
	char **brxnreactstr;        // strings for reactants [index]
	char **brxnprodstr;         // strings for products [index]
  char **brxnratestr;         // strings for reaction rates [index]
  int **brxnreact;            // reactant bng species indices [index][rct]
  int **brxnprod;             // product bng species indices [index][prd]
  int *brxnorder;             // order of bng reaction [index]
  int *brxnnprod;             // number of products of bng reaction [index]
  rxnptr *brxn;               // pointer to this reaction [index]
} *bngptr;

typedef struct bngsuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	char *BNG2path;							// path and name of BNG2.pl executable
	int maxbng;                 // maximum number of bng networks
	int nbng;                   // actual number of bng networks
	char **bngnames;            // names of bng networks
	bngptr *bnglist;            // list of bng networks
} *bngssptr;

/********************************* Graphics ********************************/

#define MAXLIGHTS 8						// must be ? GL_MAX_LIGHTS
enum LightParam {LPambient,LPdiffuse,LPspecular,LPposition,LPon,LPoff,LPauto,LPnone};

typedef struct graphicssuperstruct {
	enum StructCond condition;	// structure condition
	struct simstruct *sim;			// simulation structure
	int graphics;								// graphics: 0=none, 1=opengl, 2=good opengl
	int runmode;								// 0=Smoldyn, 1=Libsmoldyn
	int currentit;							// current number of simulation time steps
	int graphicit;							// number of time steps per graphics update
	unsigned int graphicdelay;	// minimum delay (in ms) for graphics updates
	int tiffit;									// number of time steps per tiff save
	double framepts;						// thickness of frame for graphics
	double gridpts;							// thickness of virtual box grid for graphics
	double framecolor[4];				// frame color [c]
	double gridcolor[4];				// grid color [c]
	double backcolor[4];				// background color [c]
	double textcolor[4];				// text color [c]
	int maxtextitems;						// allocated size of item list
	int ntextitems;							// actual size of item list
	char **textitems;						// items to display with text [item]
	enum LightParam roomstate;	// on, off, or auto (on)
	double ambiroom[4];					// global ambient light [c]
	enum LightParam lightstate[MAXLIGHTS];	// on, off, or auto (off) [lt]
	double ambilight[MAXLIGHTS][4];		// ambient light color [lt][c]
	double difflight[MAXLIGHTS][4];		// diffuse light color [lt][c]
	double speclight[MAXLIGHTS][4];		// specular light color [lt][c]
	double lightpos[MAXLIGHTS][4];		// light positions [lt][d]
	} *graphicsssptr;

/******************************** Simulation *******************************/

#define ETMAX 11
enum SmolStruct {SSmolec,SSwall,SSrxn,SSrule,SSsurf,SSbox,SScmpt,SSport,SSfilament,SScmd,SSsim,SScheck,SSall,SSnone};
enum EventType {ETwall,ETsurf,ETdesorb,ETrxn0,ETrxn1,ETrxn2intra,ETrxn2inter,ETrxn2wrap,ETrxn2hybrid,ETimport,ETexport};

typedef int (*diffusefnptr)(struct simstruct *);
typedef int (*surfaceboundfnptr)(struct simstruct *,int);
typedef int (*surfacecollisionsfnptr)(struct simstruct *,int,int);
typedef int (*assignmols2boxesfnptr)(struct simstruct *,int,int);
typedef int (*zeroreactfnptr)(struct simstruct *);
typedef int (*unimolreactfnptr)(struct simstruct *);
typedef int (*bimolreactfnptr)(struct simstruct *,int);
typedef int (*checkwallsfnptr)(struct simstruct *,int,int,boxptr);


typedef struct simstruct {
	enum StructCond condition;	// structure condition
	FILE *logfile;							// file to send output
	char *filepath;							// configuration file path
	char *filename;							// configuration file name
	char *flags;								// command-line options from user
	time_t clockstt;						// clock starting time of simulation
	double elapsedtime;					// elapsed time of simulation
	long int randseed;					// random number generator seed
	int eventcount[ETMAX];			// counter for simulation events
	int maxvar;									// allocated user-settable variables
	int nvar;										// number of user-settable variables
	char **varnames;						// names of user-settable variables [v]
	double *varvalues;					// values of user-settable variables [v]
	int dim;										// dimensionality of space.
	double accur;								// accuracy, on scale from 0 to 10
	double time;								// current time in simulation
	double tmin;								// simulation start time
	double tmax;								// simulation end time
	double tbreak;							// simulation break time
	double dt;									// simulation time step
	int quitatend;							// simulation quits at the end
	rxnssptr rxnss[MAXORDER];		// reaction superstructures
	rulessptr ruless;						// rule superstructure
	molssptr mols;							// molecule superstructure
	wallptr *wlist;							// list of walls
	surfacessptr srfss;					// surface superstructure
	boxssptr boxs;							// box superstructure
	compartssptr cmptss;				// compartment superstructure
	portssptr portss;						// port superstructure
	latticessptr latticess;			// lattice superstructure
	bngssptr bngss;							// bionetget superstructure
	filamentssptr filss;				// filament superstructure
	cmdssptr cmds;							// command superstructure
	graphicsssptr graphss;			// graphics superstructure
	diffusefnptr diffusefn;											// function for molecule diffusion
	surfaceboundfnptr surfaceboundfn;						// function for surface-bound molecules
	surfacecollisionsfnptr surfacecollisionsfn; // function for surface collisons
	assignmols2boxesfnptr assignmols2boxesfn;		// function that assigns molecs to boxes
	zeroreactfnptr zeroreactfn;									// function for zero order reactions
	unimolreactfnptr unimolreactfn;							// function for first order reactions
	bimolreactfnptr bimolreactfn;								// function for second order reactions
	checkwallsfnptr checkwallsfn;								// function for molecule collisions with walls

#ifdef ENABLE_PYTHON_CALLBACK
	CallbackFunc *callbacks[MAX_PY_CALLBACK];  // Python callback.
	unsigned int ncallbacks;   // number of callbacks.
	size_t simstep;
#endif

#ifdef OPTION_VCELL
	VolumeSamplesPtr volumeSamplesPtr;
	ValueProviderFactory* valueProviderFactory;
	AbstractMesh* mesh;
#endif
	} *simptr;


/*********************************** VCell *********************************/


#ifdef OPTION_VCELL

	struct CompartmentIdentifierPair {
		char name[128];
		unsigned char pixel;//the compartmentID
	};

	typedef struct VolumeSamples {
		int num[3];//number of mesh points in X, Y,Z
		double size[3];//actual size in X, Y, Z (e.g. in micron)
		double origin[3];//origin of the X, Y, Z
		unsigned char* volsamples;//compartmentID for each mesh point center
		int nCmptIDPair; // number of compartments
		CompartmentIdentifierPair* compartmentIDPairPtr;//ID vs. comptName pairs.
	}* VolumeSamplesPtr;

	class ValueProviderFactory;
	class AbstractMesh;


	class ValueProvider {
	public:
		virtual ~ValueProvider(){};
		virtual double getConstantValue()=0;
		virtual double getValue(double t, double x, double y, double z, rxnptr rxn)=0;
		virtual double getValue(double t, double x, double y, double z, rxnptr rxn, char* panelName)=0;
		virtual double getValue(double t, double x, double y, double z, surfactionptr actiondetails, char* panelName)=0;
	};

	class ValueProviderFactory {
	public:
		virtual ~ValueProviderFactory(){};
		virtual ValueProvider* createValueProvider(string& rateExp)=0;
		void setSimptr(simptr sim){this->sim = sim;}
		simptr getSimptr(){return this->sim;}
	private:
		simptr sim;
	};

	class AbstractMesh{
	public:
		virtual ~AbstractMesh(){};
		virtual void getCenterCoordinates(int volIndex, double* coords)=0;
		virtual void getDeltaXYZ(double* delta)=0;
		virtual void getNumXYZ(int* num)=0;
	};

#endif


#endif

