<p align="center"><img src ="http://www.devin.com.br/wp-content/uploads/2010/07/mysql_logo.png" /></p>
# MySQL-Stataments
Stataments para MySQL no SA-MP.
>:heavy_exclamation_mark: Após uma perda de dados, a parte do código que se destinava a leitura foi perdida, portanto este agora apenas escreve dados, não lê.  

##Funções
```pawn
// Prepared statements
Statement:stmt_prepare(ConnectionHandle, query[])

bool:stmt_execute(&Statement:statement, callback = "", auto_free_result = true)

stmt_bind_value(&Statement:statement, param, DataType:type, ...)

stmt_close(&Statement:statement)
```

##### stmt_prepare
>* Prepara um statement. Coloque pontos de interrogação como espaços reservados para valores a serem inseridos. 
>* **Parametros:**
>   * `ConnectionHandle`: ID da conexão com o banco de dados.
>   * `query[]`: Query a ser preparada.
>* **Retorno:**
>   * - ID do Statament criado.
>* **Notas:**
>   * Não coloque aspas em torno dos pontos de interrogação.
***

##### stmt_bind_value
>* Definir um valor ao ponto de interrogação.
>* **Parametros:**
>   * `&Statement:statement`: Statament em que o valor será designado.
>   * `param`: Numero do parametro a alterar.
>   * `DataType:type`: Tipo de dado a ser inserido.
>	* `...`: Dado a inserir.
>* **Retorno:**
>	* - Verdadeiro caso obtenha sucesso, falso caso contrário.
>* **Notas:**
>   * Primeiro ponto de interrogação é param 0, segundo é 1, etc.
***

##### stmt_close
>* Fecha um Statament.
>* **Parametros:**
>   * `&Statement:statement`: Statament que será fechado.
>* **Retorno:**
>	* - Verdadeiro caso obtenha sucesso, falso caso contrário.
***

##### bool:stmt_execute
>* Executa o statement. Caso a callback seja valida irá chama-la
>* **Parametros:**
>   * `&Statement:statement`: Statament que será executado.
>   * `callback[]`: Callback que irar ser chamada após a query ser realizada.
>* **Retorno:**
>   * - Verdadeiro caso obtenha sucesso, falso caso contrário.
***


###Inserindo dados
```pawn
// Prepara o statement
new Statement:stmt = stmt_prepare(Conection, "INSERT INTO mytable VALUES(?, ?, ?)");

// Seta os valores nas interrogações (Primeiro é 0, segundo é 1, etc...)
stmt_bind_value(stmt, 0, TYPE_FLOAT , 123.456);
stmt_bind_value(stmt, 1, TYPE_INT   , 1234567);
stmt_bind_value(stmt, 2, TYPE_STRING, "there's no need to \"escape\" anything!");

// Executa o statement
stmt_execute(stmt);

// Agora fecha-o
stmt_close(stmt);
```

###Créditos
* Slice criação original do SQLitei.
* Dayvison transcrição de parte do código para MySQL.
