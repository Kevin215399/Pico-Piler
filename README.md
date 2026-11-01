<h1>Pico-Piler</h1>

<h2>--- Overview ---</h2>
The Pico-Piler is a compiler designed to translate C code into Risc-V assembly. 
This is important because CPU's can't understand high level languages, and they must first be converted into assembly. 

<h2>--- Features ---</h2>
The Pico-Piler includes a variety of supported syntax:


<table>
  <tr>
    <td>
      <h3>Arithmetic</h3>
    </td>
    <td>
      <h3>Flow Control</h3>
    </td>
    <td>
      <h3>Functions</h3>
    </td>
    <td>
      <h3>Pointers</h3>
    </td>
    <td>
      <h3>Standard IO</h3>
    </td>
  </tr>

  <tr>
    <td valign="top">
      <ul>
        <li>+</li>
        <li>-</li>
        <li>*</li>
        <li>/</li>
        <li>%</li>
        <li>==</li>
        <li>!=</li>
        <li>>=</li>
        <li><=</li>
        <li><</li>
        <li>></li>
        <li>&&</li>
        <li>&</li>
        <li>||</li>
        <li>|</li>
        <li>~</li>
        <li>!</li>
      </ul>
    </td>
    <td valign="top">
      <ul>
        <li>if</li>
        <li>else</li>
        <li>else if</li>
        <li>while</li>
        <li>break</li>
        <li>return</li>
      </ul>
    </td>
    <td valign="top">
      <ul>
        <li>Declerations</li>
        <li>Recursion</li>
        <li>Parameters</li>
        <li>Return values</li>
      </ul>
    </td>
    <td valign="top">
      <ul>
        <li>Dereferencing</li>
        <li>Referencing</li>
        <li>Pointer arrays</li>
      </ul>
    </td>
    <td valign="top">
      <ul>
        <li>pinMode(pin, mode)</li>
        <li>pinToggle(pin)</li>
        <li>pinWrite(pin, value)</li>
        <li>pinRead(pin)</li>
        <li>malloc(bytes)</li>
        <li>memWrite(address, value)</li>
      </ul>
    </td>
  </tr>
</table>

<h2>--- How it works ---</h2>
Here are the steps:
<ol>
  <li>The lexer splits the user code into tokens</li>
  <li>The syntax analyzer uses a custom CFG (context free grammar) to check syntax and generate the Abstract Syntax Tree</li>
  <li>The Special parser rearranges any math equations into Post-Fix notation using shunting yard algorithm</li>
  <li>The Semantic analyzer navigates the tree and calls functions to generate the final assembly (I am planning to add type checking here too)</li>
</ol>

<h2>--- Demo ---</h2>

Showcase video:
https://www.youtube.com/watch?v=wplgGrQUDOs

<h2>--- Reflection ---</h2>
I learned a lot about how compilers work through this event, and I am very happy with my first ever compiler! I hope to add more features such as for loops, type checking, and more standard functions.
