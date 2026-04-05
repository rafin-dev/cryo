# Cryo
Cryo is a simple interpreted programming language created with the objective of learning about programming languages, thus it isn't, and never will be, a performant and production ready language.

## Introduction to Cryo


### Hello World
A simple hello world in Cryo looks like this:
```
fn main() {
   println("Helo world!");
}
```
As you can see, Cryo uses a Rust-like syntax, requiring semi-colons and brackets, and the default startup function is called 'main'. Functions are defined in a very similar way to Rust, with the exception that there are no return types.

### Variables
Variables in Cryo are declared with the var keyword
```
var myVariable;
```
Cryo variables have no static type, and follow C-like scope rules
```
{
   var myVar = 180; // This is an integer
   myvar = "But now it's a string!";
}
// The variable ran out of scope
// and doesn't exist anymore
```


## Flow Control

### if then else statements
if statements in Cryo are very similar to C
```
var condition = true;
var secondCondition = false;
if (condition) {
    println("IF");
} else if (secondCondition) {
    println("ELSE IF");
} else {
    println("ELSE");
}
```
nesting if statements can be merged without the use of a third keyword, and the expression following an if needs to result in a Boolean value

### Basic Loop
Cryo has two types of loops, the first one is simply called: loop
```
fn main() {
    loop (3) {
        println("Cryo");
    }
}
// Output:
// Cryo
// Cryo
// Cryo
```
This kind of loop simply repeats its body N times, with N being a positive integer value

### while Loop
The second type of loop in Cryo is the while Loop
```
fn main() {
    var count = 0;
    while (count < 3) {
        println("count: ", count);
        coutn += 1;
    }
}
// Output:
// count: 0
// count: 1
// count: 2
```
This type of loop repeats it's body until the condition becomes expression becomes false

## Declaring and calling functions
Functions are declared with the 'fn' keyword and don't need to be defined before they are called in the file, as long as the function exists somewhere
```
fn main() {
    greet();
}

fn greet(var name) {
    println("Hello ", name, ", it's nice to meet you!");
}

fn getName() {
    return "Random Name";
}
```
Cryo functions don't have a defined return type, so a single function can return diferent types
```
fn myFunction(var condition) {
    if (condition) {
        return "Hello"; // returns a string
    } else {
        return 0; // Returns an integer
    }
}
```