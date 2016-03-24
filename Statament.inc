#define MAX_PARAMS  		(32)
#define MAX_FIELD_NAME  	(66)
#define MAX_STATEMENTS  	(16)
#define MAX_STATEMENT_SIZE  (1024)
#define MAX_FIELDS  		(64)

#define Notice(%1)    print(!"Notice: " %1)
#define Noticef(%1)   printf("Notice: " %1)

#define Warning(%1)   print(!"Warning: " %1)
#define Warningf(%1)  printf("Warning: " %1)

#define Error(%1)     print(!"Error: " %1)
#define Errorf(%1)    printf("Error: " %1)
#define DEBUG 		(true)

#define Debug(%1)  printf("Debug: " %1)
const
	Statement:INVALID_STATEMENT = Statement:-1,
	  		  INVALID_CONNECTION = 0;


enum DataType {
	TYPE_NONE,
	TYPE_NULL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_RAW_STRING,
};

enum E_STATEMENT{
	// The ready-to-run query.
	e_szQuery[MAX_STATEMENT_SIZE + 1],
	
	// Parameter count
	e_iParams,
	
	// The parameter types.
	DataType:e_aiParamTypes[MAX_PARAMS],
	
	// Position of parameters in the query string.
	e_aiParamPositions[MAX_PARAMS],
	
	// Length of parameters in the query string.
	e_aiParamLengths[MAX_PARAMS],
	
	// The database it was created for
	e_ConnectionHandle,
};
static stock 
	gs_szBuffer[8192],
	gs_Statements[Statement:MAX_STATEMENTS][E_STATEMENT];


stock Statement:stmt_prepare(Connection, const szQuery[]) {

	new
		Statement:stStatement = INVALID_STATEMENT,
		iPos,
		i,
		iLength
	;
	
	if(Connection == INVALID_CONNECTION) {
		Error("(stmt_prepare) Conexao invalida.");
		
		return INVALID_STATEMENT;
	}
	
	// Pretty useless to prepare empty queries.
	if (!(iLength = strlen(szQuery))) {
		Error("(stmt_prepare) Query vazia.");
		
		return INVALID_STATEMENT;
	}
	
	if (iLength char > MAX_STATEMENT_SIZE) {
		Error("(stmt_prepare) Query muito longa. Aumente MAX_STATEMENT_SIZE.");
		
		return INVALID_STATEMENT;
	}
	
	// Find an empty slot in gs_Statements.
	for (i = 0; i < sizeof(gs_Statements); i++) {
		if (!gs_Statements[Statement:i][e_ConnectionHandle]) {
			stStatement = Statement:i;
			
			break;
		}
	}
	
	if (stStatement == INVALID_STATEMENT) {
		Error("(stmt_prepare) Nenhum slot encontrado livre para fazer o statament. Aumente MAX_STATEMENTS.");
		
		return INVALID_STATEMENT;
	}
	
	gs_Statements[stStatement][e_ConnectionHandle] = Connection;
	
	
	// Make sure no parameters are initialized.
	for (i = 0; i < MAX_PARAMS; i++)
		gs_Statements[stStatement][e_aiParamTypes][i] = TYPE_NONE;
	
	iPos = -1;
	i = 0;
	
	// Find all parameters
	while (-1 != (iPos = strfind(szQuery, "?", _, ++iPos))) {
		gs_Statements[stStatement][e_aiParamPositions][i] = iPos;
		gs_Statements[stStatement][e_aiParamLengths][i] = 1;
		
		if (++i >= MAX_PARAMS) {
			Error("(stmt_prepare) Limite de parametros excedido. Aumente MAX_PARAMS.");
			
			return INVALID_STATEMENT;
		}
	}
	
	gs_Statements[stStatement][e_iParams] = i;
	
	gs_Statements[stStatement][e_szQuery][0] = 0;
	
	if (ispacked(szQuery)) {
	#if DEBUG
		strunpack(gs_szBuffer, szQuery);
		
		Debug("(stmt_prepare:%d) Preparando statement com %d parametros: %s", _:stStatement, i, gs_szBuffer);
	#endif
		
		strcat(gs_Statements[stStatement][e_szQuery], szQuery, MAX_STATEMENT_SIZE);
	} else {
		Debug("(stmt_prepare:%d) Preparando statement com %d parametros: %s", _:stStatement, i, szQuery);
		
		strpack(gs_Statements[stStatement][e_szQuery], szQuery, MAX_STATEMENT_SIZE);
	}
	
	return stStatement;
}

stock bool:stmt_bind_value(&Statement:stStatement, iParam, DataType:iType, {Float,_}:...) 
{
	
	new
		iLengthDiff,
		iLength
	;
	
	if (stStatement == INVALID_STATEMENT || !(0 <= _:stStatement < sizeof(gs_Statements))) {
		Warningf("(stmt_bind_value) Statament invalido (%d).", _:stStatement);
		
		return false;
	}
	
	if (iParam >= gs_Statements[stStatement][e_iParams]) {
		Warningf("(stmt_bind_value) Parametro execendo o limite de parametros definidos, refaca o query. (%d > %d).", iParam, gs_Statements[stStatement][e_iParams]);
		
		return false;
	}
	
	// Fill gs_szBuffer with the new contents.
	gs_szBuffer[0] = 0;
	
	switch (iType) {
		case TYPE_NULL:
			goto default_case;
		
		case TYPE_INT: {
			new
				iArgValue = getarg(3)
			;
			
			if (iArgValue == cellmin)
				gs_szBuffer = !"-2147483648";
			else
				format(gs_szBuffer, sizeof(gs_szBuffer), "%d", getarg(3));
		}
		case TYPE_FLOAT:
		{	
			format(gs_szBuffer, sizeof(gs_szBuffer), "%f", getarg(3));
		}
		case TYPE_STRING:
		{
			strpack(gs_szBuffer, @arg[3], (sizeof(gs_szBuffer) - 3));

			escape_string(gs_szBuffer, "'", sizeof(gs_szBuffer) - 1);
		}
		case TYPE_RAW_STRING:
		{	
			strcat(gs_szBuffer, @arg[3]);
		}
		default:
		{
			default_case:
			strcat(gs_szBuffer, "NULL");
		}
	}
	
	iLength = strlen(gs_szBuffer);
	
	iLengthDiff = iLength - gs_Statements[stStatement][e_aiParamLengths][iParam];
	
	// Adjust the position of any params after the one being modified.
	for (new i = iParam + 1; i < gs_Statements[stStatement][e_iParams]; i++)
		gs_Statements[stStatement][e_aiParamPositions][i] += iLengthDiff;
	
	// Delete the old parameter from the query.
	strdel(gs_Statements[stStatement][e_szQuery], gs_Statements[stStatement][e_aiParamPositions][iParam], gs_Statements[stStatement][e_aiParamPositions][iParam] + gs_Statements[stStatement][e_aiParamLengths][iParam]);
	
	// Make sure we have enough space.
	if ((strlen(gs_Statements[stStatement][e_szQuery]) + iLength) char > MAX_STATEMENT_SIZE) {
		Error("(stmt_bind_value) Buffer overflow. Aumente MAX_STATEMENT_SIZE.");
		
		stmt_close(stStatement);
		
		return false;
	}
	
	// Insert the new parameter.
	strins(gs_Statements[stStatement][e_szQuery], gs_szBuffer, gs_Statements[stStatement][e_aiParamPositions][iParam], MAX_STATEMENT_SIZE);
	
	#if DEBUG
	if (ispacked(gs_szBuffer))
		strunpack(gs_szBuffer, gs_szBuffer);
	
	Debug("(stmt_bind_value:%d) Inserido novo parametro para %d em %d: %s", _:stStatement, iParam, gs_Statements[stStatement][e_aiParamPositions][iParam], gs_szBuffer);
	#endif
	
	gs_Statements[stStatement][e_aiParamLengths][iParam] = iLength;
	gs_Statements[stStatement][e_aiParamTypes][iParam] = iType;
	
	return true;
}

stock escape_string(szString[], const szEnclosing[] = "'", iSize = sizeof(szString)) {
	new
		iPos
	;
	
	while (-1 != (iPos = strfind(szString, szEnclosing, _, iPos))) {
		strins(szString, szEnclosing, iPos, iSize);
		
		iPos += 2;
	}
}

stock stmt_close(&Statement:stStatement){
	
	if (stStatement == INVALID_STATEMENT || !(0 <= _:stStatement < sizeof(gs_Statements))) {
		Noticef("(stmt_close) Statament invalido (%d).", _:stStatement);
		
		return false;
	}
	gs_Statements[stStatement][e_ConnectionHandle] = INVALID_CONNECTION;
	
	Debug("(stmt_close:%d) Statament fechado.", _:stStatement);
	
	stStatement = INVALID_STATEMENT;
	return true;
}

stock bool:stmt_execute(Statement:stStatement, callback[] = "") {

	if (stStatement == INVALID_STATEMENT || !(0 <= _:stStatement < sizeof(gs_Statements))) {
		Errorf("(stmt_execute) Statament invalido (%d).", _:stStatement);
		
		return false;
	}
	
	if (!gs_Statements[stStatement][e_ConnectionHandle]) {
		Errorf("(stmt_execute) Conexao invalida (%d).", _:stStatement);
		
		return false;
	}
	
	// Make sure all parameters have been set.
	for (new i = 0; i < gs_Statements[stStatement][e_iParams]; i++) {
		if (gs_Statements[stStatement][e_aiParamTypes][i] == TYPE_NONE) {
			Errorf("(stmt_execute) Parametro (%d) nao iniciado no statament.", i);
			
			return false;
		}
	}

	Debug("(stmt_execute:%d) Executando statement.", _:stStatement);
	strunpack(gs_szBuffer, gs_Statements[stStatement][e_szQuery]);
	new
		dbrResult = mysql_pquery(gs_Statements[stStatement][e_ConnectionHandle], gs_szBuffer, callback)
	;
	#if DEBUG

	printf("Query executada [%s]", gs_szBuffer);
	#endif
	

	if (dbrResult == 0)
		return false;
	return true;
}
