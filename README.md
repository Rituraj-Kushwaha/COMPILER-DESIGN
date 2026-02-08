# Compiler Design Lab Experiments

This folder contains implementations for 15 Compiler Design lab experiments. Each section below includes the experiment title, the corresponding source file, and a placeholder for inserting a practical code execution screenshot.

## Lab 1 - Implementation of Lexical Analyzer

- Description: A lexical analyzer (lexer) is the first phase of a compiler. It scans source code character by character, groups them into tokens (keywords, identifiers, literals, symbols), ignores whitespace/comments, and reports lexical errors. These tokens are then passed to the parser.

- Screenshot:
  
   <img width="364" height="107" alt="1" src="https://github.com/user-attachments/assets/81d0a56f-770d-4afb-8f52-a5df8548bc39" />



## Lab 2 - Conversion from Regular Expression to NFA

- Description: Builds a nondeterministic finite automaton (NFA) from a regular expression, typically using Thompson's construction, to represent pattern-matching behavior.

- Screenshot:

  <img width="335" height="524" alt="image" src="https://github.com/user-attachments/assets/bda24a18-4136-48ba-a07e-c2aa275bd3de" />

  

## Lab 3 - Conversion from NFA to DFA

- Description: Converts an NFA to an equivalent deterministic finite automaton (DFA) using the subset construction to remove nondeterminism.

- Screenshot:

  <img width="572" height="239" alt="image" src="https://github.com/user-attachments/assets/400531db-2fb2-4886-aaa4-8ae733f6d192" />



## Lab 4 - Elimation of Ambiguity, Left Recursion and Left Factoring

- Description: Demonstrates grammar transformations to reduce ambiguity and make grammars suitable for predictive parsing by removing left recursion and applying left factoring.

- Screenshot:

  <img width="428" height="681" alt="image" src="https://github.com/user-attachments/assets/19482153-0ed3-49ca-9cfa-881c706bfa25" />

  

## Lab 5 - FIRST AND FOLLOW computation

- Description: Computes FIRST and FOLLOW sets for grammar symbols, which are essential for LL(1) parsing table construction.

- Screenshot:

  <img width="436" height="287" alt="image" src="https://github.com/user-attachments/assets/d4050b5c-d2be-49ea-ba66-fb1b81f94076" />

  

## Lab 6 - Predictive Parsing Table

- Description: Builds an LL(1) predictive parsing table from a grammar using FIRST and FOLLOW sets.

- Screenshot:

  <img width="784" height="302" alt="image" src="https://github.com/user-attachments/assets/817c1d5d-36bd-484d-a2c5-91bff74c34c9" />

 

## Lab 7 - Shift Reduce Parsing

- Description: Shows bottom-up parsing using shift and reduce actions to recognize valid strings in a grammar.

- Screenshot:

  <img width="544" height="373" alt="image" src="https://github.com/user-attachments/assets/368391a1-5a2f-4f1e-8d03-fce73716a0ec" />

 

## Lab 8 - Computation of LEADING AND TRAILING

- Description: Computes LEADING and TRAILING sets for operators and terminals, useful in operator-precedence parsing.

- Screenshot:

  <img width="462" height="288" alt="image" src="https://github.com/user-attachments/assets/abaf1081-dffb-4ad9-8e8f-51464c0fab5e" />

 

## Lab 9 - Computation of LR (0) items

- Description: Constructs the canonical collection of LR(0) items and transitions used in LR parsing.

- Screenshot:

  <img width="379" height="787" alt="image" src="https://github.com/user-attachments/assets/c3218910-1bf2-415e-a0b8-c4e6ec94fe04" />

  

## Lab 10 - Intermediate code generation: Postfix, Prefix

- Description: Converts infix expressions into postfix and prefix forms as a basis for intermediate representation.

- Screenshot:

  <img width="376" height="85" alt="image" src="https://github.com/user-attachments/assets/19fa9275-a805-480b-987f-09eb6c4d0deb" />

 

## Lab 11 - Intermediate code generation: Quadruple, Triple, Indirect triple

- Description: Generates intermediate representations (quadruples, triples, and indirect triples) for expressions.

- Screenshot:

  <img width="345" height="332" alt="image" src="https://github.com/user-attachments/assets/b33eca45-0488-411b-bec5-b64e51916f85" />

  

## Lab 12 - A simple code Generator

- Description: Generates basic three-address code and simple target-like instructions from arithmetic expressions.

- Screenshot:

  <img width="400" height="338" alt="image" src="https://github.com/user-attachments/assets/097fdaab-1aed-414b-b130-e10b0b601cdd" />

  

## Lab 13 - Implementation of DAG

- Description: Builds a DAG for expressions to identify common subexpressions and enable optimization.

- Screenshot:

  <img width="401" height="85" alt="image" src="https://github.com/user-attachments/assets/d44a2dce-ae3a-4286-850e-5ba298603aea" />

  

## Lab 14 - Implementation of Global Data Flow Analysis

- Description: Performs a classic data flow analysis (e.g., reaching definitions) to compute IN/OUT sets for basic blocks.

- Screenshot:

  <img width="577" height="548" alt="image" src="https://github.com/user-attachments/assets/da5c72c2-9062-4b3e-9cf7-c4d3acb78b70" />

  

## Lab 15 - Implement any one storage allocation strategies (heap, stack, static)

- Description: Simulates a storage allocation strategy (here, heap first-fit) and shows how blocks are allocated and freed.

- Screenshot:

  <img width="349" height="699" alt="image" src="https://github.com/user-attachments/assets/def35ef9-6132-4f5c-a7c5-f1367bbac6a0" />










