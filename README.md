<h1>Pico-Piler</h1>

<h2>--- Overview ---</h2>
The Pico-Piler is a compiler designed to translate C code into Risc-V assembly. 
This is important because CPU's can't understand high level languages, and they must first be converted into assembly. 
It runs on a singular Pico 2

\
\
<img width="636" height="356" alt="Screenshot 2026-10-31 233343" src="https://github.com/user-attachments/assets/fcac752c-2ce4-4c9a-aeef-c30c1d580306" />
\
For more info, check out this visual demo: https://www.youtube.com/watch?v=wplgGrQUDOs

<h2>--- How to run ---</h2>

A) On the online simulator
<br>
    &emsp;Click the green play button. A console should open, if not click the bottom right corner.<br>
    &emsp;Resize the console to better see the prompts

B) Using a Pico 2
<br>
    &emsp;Plug in a Pico 2 in bootsel mode, and drag the UF2 file to the pico drive. <br>
    &emsp;Open a console with Pico SDK or an IDE of your choice and follow the prompts
<br><br>

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

<h2>--- Reflection ---</h2>
I learned a lot about how compilers work through this event, and I am very happy with my first ever compiler! I hope to add more features such as for loops, type checking, and more standard functions.
