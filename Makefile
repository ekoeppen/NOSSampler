# --- Common variables  -------------------------------------------------------------

INCLUDES = 	-i "{DDK_Includes-dir}" ¶
	-i "{DDK_Includes-dir}CLibrary:" ¶
	-i "{DDK_Includes-dir}CommAPI:" ¶
	-i "{DDK_Includes-dir}Communications:" ¶
	-i "{DDK_Includes-dir}Frames:" ¶
	-i "{DDK_Includes-dir}HAL:" ¶
	-i "{DDK_Includes-dir}OS600:" ¶
	-i "{DDK_Includes-dir}Power:" ¶
	-i "{DDK_Includes-dir}Toolbox:" ¶
	-i "{DDK_Includes-dir}UtilityClasses:" ¶
	-i "{DDK_Includes-dir}PCMCIA:" ¶
	-i "{NCT_Includes}" ¶
	-i "{NCT_Includes}Frames:" ¶
	-i "{NCT_Includes}Utilities:"
LIBS = "{NCT_Libraries}MP2x00US.a.o"

MAKEFILE     = Makefile
Objects-dir  = :{NCT-ObjectOut}:
LIB          = {NCT-lib} {NCT-lib-options} {LocalLibOptions} 
LINK         = {NCT-link}
LINKOPTS     = {NCT-link-options} -rel {LocalLinkOptions} 
Asm          = {NCT-asm} {NCT-asm-options} {LocalAsmOptions}
CFront       = {NCT-cfront} {NCT-cfront-options} {LocalCFrontOptions}
CFrontC      = {NCT-cfront-c} {NCT-cfront-c-options} {LocalCfrontCOptions}
C            = {NCT-ARMc} {NCT-ARMc-options} 
ARMCPlus     = {NCT-ARMCpp} {NCT-ARMCpp-options} {LocalARMCppOptions}
ProtocolOptions = -package
Pram         = {NCT-pram} {NCT-pram-options} {LocalPRAMOptions} 
SETFILE      = {NCT-setfile-cmd}
SETFILEOPTS  = 
LocalLinkOptions = -dupok -debug
LocalARMCppOptions = -cfront -W -gf
LocalCfronttOptions = 
LocalCfrontCOptions = -W 
LocalCOptions = -d forARM 
LocalPackerOptions =  -packageid 'noss' -copyright 'Copyright (c) 2011 Eckhart Kšppen'
COUNT        = Count
COUNTOPTS    = 
CTAGS        = CTags
CTAGSOPTS    = -local -update
DELETE       = Delete
DELETEOPTS   = -i
FILES        = Files
FILESOPTS    = -l
LIBOPTS      = 
PRINT        = Print
PRINTOPTS    = 
REZ          = Rez

AOptions = -i "{DDK_Includes-dir}"

POptions = -i "{DDK_Includes-dir}"

ROptions = -i "{DDK_Includes-dir}" -a

COptions = {INCLUDES} {NCT_DebugSwitch} {LocalCOptions}

ASFLAGS = "{NCT-asm-options}"

# --- Project Files ----------------------------------------------------------------

all Ä NOSSampler.ntkc

EXPORTS = Exports.exp
SRCS = Main.cp Logger.cp {EXPORTS}
OBJS = Main.cp.o Logger.cp.o Exports.exp.o

NOSSampler.ntkc Ä {OBJS}
	{NCT-link} {LINKOPTS} -dupok -o {Targ} {Deps} {LIBS}
	Rename -y {Targ} NOSSampler.sym
	{NCT-AIFtoNTK} {LocalAIFtoNTKOptions} -via "{EXPORTS}" -o  {Targ} NOSSampler.sym
	{SETFILE} {SETFILEOPTS} {Targ}

# --- Generic rules ----------------------------------------------------------------

clean	Ä
	{DELETE} {DELETEOPTS} {OBJS} {MODULE} {SYMFILE} {AIFFILE}

.cp.o		Ä		.cp
	{ARMCPlus}	{depDir}{Default}.cp {COptions} -o {targDir}{Default}.cp.o

.cf.o		Ä		.cf
	{CFront} {depDir}{Default}.cf {COptions} {NCT-cfront-redirection} "{{CPlusScratch}}"X{Default}.cf -o {targDir}{Default}.cf.o
	{CFrontC} "{{CPlusScratch}}"X{Default}.cf -o {targDir}{Default}.cf.o  ; Delete -i "{{CPlusScratch}}"X{Default}.cf

.c.o	    Ä		.c
	{C} {depDir}{Default}.c -o {targDir}{Default}.c.o {COptions}

.exp.o		Ä		.exp
	"{NCTTools}"NCTBuildMain	{depDir}{Default}.exp "{{CPlusScratch}}"{Default}.exp.main.a
	{Asm}	"{{CplusScratch}}"{Default}.exp.main.a -o {targDir}{Default}.exp.o ; Delete -i "{{CPlusScratch}}"{Default}.exp.main.a

.a.o		Ä		.a
	{Asm}	{depDir}{Default}.a  -o {targDir}{Default}.a.o {AOptions}

.h.o		Ä		.h
	ProtocolGen -InterfaceGlue {depDir}{Default}.h {COptions} -stdout > "{{CPlusScratch}}"{Default}.glue.a
	{Asm} "{{CPlusScratch}}"{Default}.glue.a -o {targDir}{Default}.h.o ; Delete -i "{{CPlusScratch}}"{Default}.glue.a

.impl.h.o	Ä		.impl.h
	ProtocolGen -ImplementationGlue {depDir}{Default}.impl.h {ProtocolOptions} {COptions} -stdout >"{{CPlusScratch}}"{Default}.impl.a
	{Asm} "{{CPlusScratch}}"{Default}.impl.a -o {targDir}{Default}.impl.h.o ; Delete -i "{{CPlusScratch}}"{Default}.impl.a
