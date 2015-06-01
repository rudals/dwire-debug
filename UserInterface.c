/// UserInterface.c

//        User interface
//
//        Reads lines, parses them into commands and parameters, and calls the
//        appropriate handler procedures.





int QuitRequested = 0;

void PCommand()      {PC = ReadNumber(1);}
void Go()            {Wsl("Going, going, ...., gone.");}
void QuitCommand()   {QuitRequested = 1;}
void TraceCommand()  {DwTrace();}
void FailCommand()   {Fail("FailCommand ...");}
void HelpCommand();
void EmptyCommand()  {Sb(); if (!DwEoln()) {HelpCommand();}}


struct {char *name; char *help; int requiresConnection; void (*handler)();} Commands[] = {
  {"d",           "Dump data bytes",        1, DumpDataBytesCommand},
  {"dw",          "Dump data words",        1, DumpDataWordsCommand},
  {"g",           "Go",                     1, Go},
  {"p",           "PC set / query",         1, PCommand},
  {"q",           "Quit ",                  0, QuitCommand},
  {"r",           "Registers",              1, RegistersCommand},
  {"s",           "Stack",                  1, StackCommand},
  {"t",           "Trace",                  1, TraceCommand},
  {"u",           "Unassemble",             1, UnassembleCommand},
  {"h",           "Help",                   0, HelpCommand},
  {"reset",       "Reset processor",        1, DwReset},
  {"serialdump",  "(dwdebug debugging)",    1, SerialDump},
  {"help",        "Help",                   0, HelpCommand},
  {"device",      "Device connection port", 0, DeviceCommand},
  {"",            0,                        0, EmptyCommand}
};


enum {unconnected, connected, running} State = unconnected;


void HelpCommand() {
  Wsl("dwdebug commands:\n");
  for (int i=0; Commands[i].help; i++) {
    Ws("  "); Ws(Commands[i].name); Ws(" - "); Wsl(Commands[i].help);
  }
}


void HandleCommand(const char *cmd) {
  for (int i=0; i<countof(Commands); i++) {
    if (!strcmp(cmd, Commands[i].name)) {
      if (State == unconnected  &&  Commands[i].requiresConnection) {ConnectSerialPort();}
      Commands[i].handler();
      return;
    }
  }
  Ws("Unrecognised command: '"); Ws(cmd); Wsl("'.");
}


void ParseAndHandleCommand() {
  char command[20];

  Sb(); if (NextCh() == ';') {SkipCh(); Sb();}

  if (Eof()) {if (IsUser(Input)) {Wl();}  QuitCommand();}
  else       {Ra(ArrayAddressAndLength(command)); HandleCommand(command);}

  SkipWhile(NotDwEoln); if (Eoln()) {SkipEoln();} else {SkipCh();}
}


void DisassemblyPrompt() {
  u8 buf[4];  // Enough for a 2 word instruction
  Wx(PC, 4); Ws(": ");  // Word address, e.g. as used in pc and bp setting instructions
  DwReadFlash(PC<<1, 4, buf);
  Uaddr = PC + DisassembleInstruction(PC, buf);
  Wt(40);
}


void Prompt() {
  if (BufferTotalContent() == 0  &&  IsUser(Input)) {
    if (!Prompted) {
      switch(State) {
        case unconnected: Ws("Unconnected.");   break;
        case connected:   DisassemblyPrompt();  break;
        case running:     Ws("Running.");       break;
      }
    }
    Wt(40); Ws("> "); Flush();
  } else {
    if (Prompted) {Wl();}
  }
  Prompted = 0;
}


void UI() {

  while (1) {
    if (QuitRequested) return;
    if (setjmp(FailPoint)) {Sl();}
    Prompt();
    ParseAndHandleCommand();
    if (State == unconnected  &&  SerialPort) {State = connected;}
  }


  //while (1) {
  //
  //  if (QuitRequested) return;
  //
  //  if (BufferTotalContent() == 0  &&  IsUser(Input)) {
  //    if (!Prompted) {Prompt();}
  //    Wt(40); Ws("> "); Flush();
  //  } else {
  //    if (Prompted) {Wl();}
  //  }
  //  Prompted = 0;
  //
  //  Sb(); if (NextCh() == ';') {SkipCh(); Sb();}
  //
  //  if (Eof()) {if (IsUser(Input)) {Wl();} break;}
  //  else       {Ra(ArrayAddressAndLength(command)); HandleCommand(command);}
  //
  //  SkipWhile(NotDwEoln); if (Eoln()) {SkipEoln();} else {SkipCh();}
  //}
}