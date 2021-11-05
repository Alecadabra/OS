# Operating Systems

Part of my Computer Science degree at Curtin University in 2020. My final mark was 82%.

<table>
  <thead>
    <tr>
      <th colspan="6">Curtin University • BSc Computer Science</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td colspan="2">2019 • 1st Year</td>
      <td colspan="2">2020 • 2nd Year</td>
      <td colspan="2">2021 • 3rd Year</td>
    </tr>
    <tr>
      <td>Sem 1</td>
      <td>Sem 2</td>
      <td>Sem 1</td>
      <td>Sem 2</td>
      <td>Sem 1</td>
      <td>Sem 2</td>
    </tr>
    <tr>
      <td rowspan="3">
        <a href="https://github.com/Alecadabra/OOPD" target="_blank" rel="noopener noreferrer">
        <abbr title="Object Oriented Program Design">OOPD</abbr></a></a>
      </td>
      <td rowspan="3">
        <a href="https://github.com/Alecadabra/DSA" target="_blank" rel="noopener noreferrer">
          <abbr title="Data Structures and Algorithms">DSA</abbr></a>
        <br>
        <a href="https://github.com/Alecadabra/UCP" target="_blank" rel="noopener noreferrer">
          <abbr title="Unix and C Programming">UCP</abbr></a>
      </td>
      <td rowspan="3">
        <a href="https://github.com/Alecadabra/OOSE" target="_blank" rel="noopener noreferrer">
          <abbr title="Object Oriented Software Engineering">OOSE</abbr></a></a>
        <br>
          <abbr title="Operating Systems"><b>OS</b></abbr>
      </td>
      <td rowspan="3">
        <a href="https://github.com/Alecadabra/CG" target="_blank" rel="noopener noreferrer">
          <abbr title="Computer Graphics">CG</abbr></a></a>
        <br>
        <a href="https://github.com/Alecadabra/MAD" target="_blank" rel="noopener noreferrer">
          <abbr title="Mobile Application Development">MAD</abbr></a></a>
        <br>
        <a href="https://github.com/Alecadabra/PL" target="_blank" rel="noopener noreferrer">
          <abbr title="Programming Languages">PL</abbr></a></a>
      </td>
      <td rowspan="3">
        <a href="https://github.com/Alecadabra/HCI" target="_blank" rel="noopener noreferrer">
          <abbr title="Machine Perception">HCI</abbr></a></a></td>
      <td rowspan="3">
        <a href="https://github.com/Alecadabra/MP" target="_blank" rel="noopener noreferrer">
          <abbr title="Machine Perception">MP</abbr></a></a>
        <br>
        <a href="https://github.com/Alecadabra/SEC" target="_blank" rel="noopener noreferrer">
          <abbr title="Software Engineering Concepts">SEC</abbr></a></a>
      </td>
    </tr>
    <tr>
    </tr>
    <tr>
    </tr>
  </tbody>
</table>

**Syllabus**

> The operating systems are a critical part of any computer systems, and therefore
it is important for all computing students to have sufficient knowledge of 
operating systems that evolve with the development of computer technologies. In 
this unit students will learn the following main OS components: process, thread, 
and their communication and synchronizations; CPU scheduling algorithms; 
deadlock detection, prevention, and avoidance; memory management, including 
memory allocation, memory paging and segmentation, and virtual memory; file 
system interface, implementation, and access methods; disk structure, 
scheduling, management and reliability; I/O management; and protection and 
security concepts and implementations.

## [Assignment](Assignment)

A long term coding assignment about thread/process synchronisation in C.
Simulates 3 lifts serving lift requests created my a requester in a 
bounded-buffer producer/consumer problem. Part A of the assignment uses threads
to run the multiple lifts and reqeuester using POSIX Pthreads and Pthread's
mutex and cond for mutual exclusion, and Part B uses forked processes with POSIX
memory sharing and semaphores for mutual exclsion. See readme inside for details
on how to run/compile.
