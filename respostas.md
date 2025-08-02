# Respostas - Explorando Representações Intermediárias com LLVM IR

## 2\. Código C e Geração de IR

### Como estão representadas as funções `soma`, `multiplica` e `calcula` em IR?

**Resposta:**
As funções são representadas com a sintaxe `define` seguida do tipo de retorno, nome e parâmetros:

* `soma`: `define dso_local i32 @soma(i32 noundef %0, i32 noundef %1)`
* `multiplica`: `define dso_local i32 @multiplica(i32 noundef %0, i32 noundef %1)`
* `calcula`: `define dso_local i32 @calcula(i32 noundef %0)`

Todas seguem o padrão LLVM IR:

* `i32`: tipo inteiro de 32 bits
* `@nome`: símbolo global da função
* `%0, %1`: parâmetros em formato SSA (Static Single Assignment)
* `noundef`: atributo indicando que o valor não é undefined

### O que aparece no IR que representa a condição `if (valor > 10)`?

**Resposta:**
A condição `if (valor > 10)` é representada por duas instruções:

``` llvm
%5 = icmp sgt i32 %4, 10    ; compara se %4 > 10 (signed greater than)
br i1 %5, label %6, label %9 ; branch condicional baseado no resultado
```

* `icmp sgt`: instrução de comparação "signed greater than"
* `br i1`: instrução de branch condicional que pula para o label %6 se verdadeiro, senão para %9

### Como são representadas as chamadas às funções auxiliares em IR?

**Resposta:**
As chamadas de função usam a instrução `call`:

``` llvm
%8 = call i32 @multiplica(i32 noundef %7, i32 noundef 2)
%11 = call i32 @soma(i32 noundef %10, i32 noundef 5)
```

O formato é: `%registrador_virtual = call tipo_retorno @nome_funcao(tipo arg1, tipo arg2, ...)`

## 3\. Modificação do Código

### Como o `if (temp % 2 == 0)` aparece no IR?

**Resposta:**
A instrução de desvio e a condição são representadas por três instruções sequenciais:

``` llvm
%8 = srem i32 %7, 2        ; calcula resto da divisão por 2
%9 = icmp eq i32 %8, 0     ; compara se resto é igual a 0
br i1 %9, label %10, label %13  ; branch condicional
```

### Como o operador `%` (módulo) é representado no LLVM IR?

**Resposta:**
O operador módulo `%` é representado pela instrução `srem` (signed remainder):

É usado em operacoes\_mod.ll:

``` llvm
%8 = srem i32 %7, 2
```

* `srem`: signed remainder (resto da divisão com sinal)
* Calcula `%7 % 2` e armazena o resultado em `%8`

### Quais são os blocos básicos criados pela nova lógica condicional?

**Resposta:**
A função `calcula` possui **4 blocos básicos**:

1. Bloco 1: Chama `soma`, calcula módulo e faz comparação
2. Bloco 10: Executa quando número é par → chama `multiplica`
3. Bloco 13: Executa quando número é ímpar → chama `divide`
4. Bloco 16: Bloco de convergência → retorna resultado final

## 4\. Otimização com `opt`

### Que mudanças ocorreram na função `main` após a otimização?

**Resposta:**
Após aplicar `opt -O2`, as principais mudanças na função `main` foram:

**Antes da otimização:**

``` llvm
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %3 = call i32 @calcula(i32 noundef 7)
  store i32 %3, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  %5 = call i32 (ptr, ...) @__mingw_printf(ptr noundef @.str, i32 noundef %4)
  ret i32 0
}
```

**Depois da otimização:**

``` llvm
define dso_local noundef i32 @main() local_unnamed_addr #1 {
  %1 = tail call i32 @calcula(i32 noundef 7)
  %2 = tail call i32 (ptr, ...) @__mingw_printf(ptr noundef nonnull @.str, i32 noundef %1) #3
  ret i32 0
}
```

**Mudanças observadas:**

1. Eliminação de variáveis locais ao remover operações `alloca`, `store` e `load`
2. Tail call optimization, convertendo chamadas em `tail call`
3. Resultado de `calcula` usado diretamente no `printf`

### Alguma função foi *inlined* (inserida diretamente)? Como identificar?

**Resposta:**
Não, nenhuma função foi completamente inlined, pois as funções foram mantidas separadas (todas as definições `define` ainda existem) e o atributo **noinline** ainda está presente, impedindo inlining completo.
Se houvesse inlining, O código da função seria copiado para o local da chamada.

### Alguma variável intermediária foi eliminada? Por quê?

**Resposta:**
Sim, algumas variáveis intermediárias foram eliminadas através de(o):

**1\. Eliminação de allocation\-to\-register:**

``` llvm
// Antes: operações de memória desnecessárias
%3 = alloca i32, align 4
%4 = alloca i32, align 4
store i32 %0, ptr %3, align 4
store i32 %1, ptr %4, align 4
%5 = load i32, ptr %3, align 4
%6 = load i32, ptr %4, align 4
%7 = add nsw i32 %5, %6

// Depois: operação direta
%3 = add nsw i32 %1, %0
```

**2\. Uso de PHI nodes:**

``` llvm
// Convergência otimizada com PHI
%.0 = phi i32 [ %4, %3 ], [ %6, %5 ]
```

**O porquê disso acontecer:**
O LLVM detectou que variáveis locais podiam ser mantidas em registradores em vez de memória, eliminando operações desnecessárias de store/load, ou seja, ocorreu uma conversão de allocation-to-register: de alocação na stack para uso direto de registradores virtuais

## 5\. Visualizando o Grafo de Fluxo de Controle \(CFG\)

### Quantos blocos básicos você consegue identificar na função `calcula`?

**Resposta:**
A função `calcula` possui 4 blocos básicos:

1. Bloco 1: Entrada, declarações, chamada `soma(valor, 3)`, cálculo módulo e comparação
2. Bloco 10: Ramo "par" - chama `multiplica(temp, 4)`
3. Bloco 13: Ramo "ímpar" - chama `divide(temp, 2)`
4. Bloco 16: Convergência - carrega resultado e retorna

### Quais são os caminhos possíveis a partir da condição `if (temp % 2 == 0)`?

**Resposta:**
Existem **2 caminhos possíveis**:

**Caminho 1 (temp é par):**
Bloco 1 → Bloco 10 → Bloco 16

**Caminho 2 (temp é ímpar):**
Bloco 1 → Bloco 13 → Bloco 16

Ambos os caminhos convergem no Bloco 16 para retornar o resultado.

### O fluxo de controle inclui **blocos de erro** ou **casos não triviais** (e.g., retorno precoce)?

**Resposta:**
Sim, o código possui tratamento de erro e retorno precoce na função `divide`:

A função `divide` tem 4 blocos com tratamento de divisão por zero:

* Bloco 2: Verifica se `y == 0`
* Bloco 8: Caso de erro - retorna 0 quando divisor é zero (early return/retorn precoce)
* Bloco 9: Caso normal - executa divisão `x / y`
* Bloco 13: Convergência - retorna resultado

### Há blocos com apenas instruções de salto? O que você imagina que isso indica?

**Resposta:**
Não, analisando o CFG das funções, não há blocos que contenham apenas instruções de salto (branch/jump/return). Todos os blocos possuem pelo menos uma operação para computar algum valor antes do salto.

**O que isso indica:**

1. **Blocos bem formados**: Cada bloco básico tem uma responsabilidade computacional específica
2. **Código estruturado**: Não há blocos que não fazem nada de útil (candidatos à eliminação), como blocos não-alcançáveis ou redundantes