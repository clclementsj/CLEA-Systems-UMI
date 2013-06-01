#include <dwgpart.h>

char LogFileName[MAX_FILE_NAME_LEN];
FILE *LFPtr = (FILE *)NULL;
char IPBNum[IPB_NUM_LEN];

int main(int argc, char *argv[])
{
     int FNameArg = 0; /* Filename argument number */

     /*
      * Check arguments
      */
     if (!checkArguments(argc, argv, &FNameArg))

          /*
           * Process the input file
           */
          return processFile(argv[FNameArg]);
     else
          return ERROR;
}

/*
 * checkArguments (int NumArgs, char *Arguments[], int *FNArg)
 * 
 * NumArgs = Number of arguments passed in
 * Arguments = Array of arguments
 * FNArg = Pointer to filename argument variable
 *
 */
int checkArguments(int NumArgs, char *Arguments[], int *FNArg)
{
     int ArgCount = 0; /* Counter */
     int RCode = NO_ERROR; /* Retun Code */
     

     *FNArg = 0;
     LogFileName[0] = (char)NULL;

     /*
      * Expect 5 or 7 arguments
      */
     if (NumArgs < 5 || NumArgs > 7)
          RCode--;

     /*
      * Work through each argument to determine debug mode and
      * file name specified.
      */
     while (!RCode && (++ArgCount < NumArgs))
     {
          /*
           * If debug mode flag 
           */
          if (!strcmp(Arguments[ArgCount], "-L"))
          {
               /*
                * If debug mode flag already specified, error
                */
               if (LogLevel > LOG_ERROR)
                    RCode--;
               /*
                * If no debug level is specified
                */
               else if (!Arguments[ArgCount + 1])
                    RCode--;
               else
               {
                    errno = 0;
                    LogLevel = atoi(Arguments[++ArgCount]);

                    /*
                     * If error converting debug level
                     */
                    if (errno)
                         RCode--;
               }
          }

          /*
           * If filename flag specified
           */
          else if (!strcmp(Arguments[ArgCount], "-f"))
          {
               /*
                * If filename already specified, error
                */
               if (*FNArg)
                    RCode--;

               /*
                * If no filename is given, error
                */
               else if (!Arguments[++ArgCount])
                    RCode--;
               else
                    *FNArg = ArgCount;
          }
          /*
           * If logfilename flag specified
           */
          else if (!strcmp(Arguments[ArgCount], "-l"))
          {
               /*
                * If logfilename already specified, error
                */
               if (strlen(LogFileName))
                    RCode--;

               /*
                * If no logfilename is given, error
                */
               else if (!Arguments[++ArgCount])
                    RCode--;
               else
                    strcpy(LogFileName, Arguments[ArgCount]);
          }
          else
               RCode--;
     }

     /*
      * If bad arguments where specified, say so
      */
     if (RCode)
          printf("Usage: %s [-L LogLevel] -f filename -l logfilename\n", Arguments[0]);

     return RCode;
}


/*
 * processFile (char *FileName)
 *
 * FileName - Pointer to filename
 *
 */
int processFile (char *FileName)
{
     FILE *InputFP = (FILE *)NULL;
     FILE *SQLFP = (FILE *)NULL;
     FILE *CandidatesFP = (FILE *)NULL;
     FILE *RejectsFP = (FILE *)NULL;
     char SQLFileName[MAX_FILE_NAME_LEN];
     char CandidatesFileName[MAX_FILE_NAME_LEN];
     char RejectsFileName[MAX_FILE_NAME_LEN];
     char *ptrFWD;
     int RCode = NO_ERROR;
     int RecordDone = NO;
     int len;
     char *ptrRecord = (char *)NULL;
     char *IPBPtr;
     char *ptr = "|";
     char *token;
     char *BoundingTag = "<tags>";
     char *PartNumOpenTag = "<pn>", *PartNumCloseTag = "</pn>";
     char *DwgNumOpenTag = "<dn>", *DwgNumCloseTag = "</dn>";
     char *DNCOpenTag = "<dncage>", *DNCCloseTag = "</dncage>";
     char *PNCOpenTag = "<pncage>", *PNCCloseTag = "</pncage>";
     char *PTOpenTag = "<tagtype>", *PTCloseTag = "</tagtype>";
     char *TSOpenTag = "<tagstatus>", *TSCloseTag = "</tagstatus>";
     char *TagList = (char *)NULL;
     char *DwgNum = (char *)NULL;
     char *PartNum = (char *)NULL;
     char *PNCage = (char *)NULL;
     char *PartType = (char *)NULL;
     char *DNCage = (char *)NULL;
     char *TagStatus = (char *)NULL;
     time_t Time;
     char LogBuffer[MAX_BUFFER_SIZE];
     char *LogTag = "processFile";
     char TmpBuffer[MAX_BUFFER_SIZE];



     if (LogLevel > LOG_WARN)
     {
          sprintf(LogBuffer, "Opening file <%s>", FileName);
          Log(LogTag, LogBuffer);
     }

     /*
      * Open the file
      */
          
     if ((InputFP = fopen(FileName, "r")))
     {

          if (LogLevel > LOG_INFO)
          {
               sprintf(LogBuffer, "<%s> opened", FileName);
               Log(LogTag, LogBuffer);
          }

          /*
           * While you haven't reach the end of the file
           */
          while (!feof(InputFP))
          {
               RecordDone = NO;

Log(LogTag, "Calling readLine()...");
               /*
                * Read Part record
                */
               if (readLine(InputFP, &ptrRecord))
               {
                    if (LogLevel > LOG_ERROR)
                         Log(LogTag, "Error reading Record");

                    RecordDone++;
               }
               else if (feof(InputFP))
               {
                    if (LogLevel > LOG_INFO)
                         Log(LogTag, "End of file reached");

                    RecordDone++;
               }
               else if (LogLevel > LOG_INFO)
               {
                    sprintf(LogBuffer, "Record is: <%s>", ptrRecord);
                    Log(LogTag, LogBuffer);
               }

               if (!RecordDone && !strncmp(ptrRecord, "01", 2))
               {
                    if (LogLevel > LOG_INFO)
                    {
                        sprintf(LogBuffer, "IPB Record is: <%s>", ptrRecord);
                        Log(LogTag, LogBuffer);
                    }

                    strcpy(TmpBuffer, ptrRecord);

                    IPBPtr = strtok(ptrRecord, ptr);
                    IPBPtr = strtok(NULL, ptr);
                    IPBPtr = strtok(NULL, ptr);
                    IPBPtr = strtok(NULL, ptr);

                    strcpy(IPBNum, IPBPtr);

                    /*
                     * Make sure IPBNum is all upper case
                     */
                    convertToUpperCase(IPBNum);

                    if (LogLevel > LOG_INFO)
                    {
                         sprintf(LogBuffer, "IPB Number = <%s>", IPBNum);
                         Log(LogTag, LogBuffer);
                    }

                    if (!(ptrFWD = getenv("FILE_WORK_DIR")))
                    {
                         sprintf(LogBuffer, "Error: Environment variable FILE_WORK_DIR not defined");
                         Log(LogTag, LogBuffer);

                         RCode--;

                    }
                    else if (LogLevel > LOG_INFO)
                    {
                         sprintf(LogBuffer, "Environment variable FILE_WORK_DIR = <%s>", ptrFWD);
                         Log(LogTag, LogBuffer);
                    }

                    if (!RCode)
                    {
                         strcpy(SQLFileName, ptrFWD);
                         strcat(SQLFileName,"/");
                         strcat(SQLFileName, IPBNum);
                         strcat(SQLFileName, ".sql");

                         if (LogLevel > LOG_INFO)
                         {
                              sprintf(LogBuffer, "Opening SQL file <%s>", SQLFileName);
                              Log(LogTag, LogBuffer);
                         }

                         if (!(SQLFP = fopen(SQLFileName, "w")))
                         {
                              strcpy(LogBuffer, strerror(errno));
                              Log(LogTag, LogBuffer);
                              RCode--;
                         }
                         else
                         {
                              if (LogLevel > LOG_INFO)
                              {
                                   sprintf(LogBuffer, "SQL file <%s> opened.", SQLFileName);
                                   Log(LogTag, LogBuffer);
                              }

                              fprintf(SQLFP, "SET SERVEROUTPUT ON\nSET FEEDBACK OFF\n");
                              fflush(SQLFP);
                         }
                    }

                    if (!RCode)
                    {
                         strcpy(CandidatesFileName, ptrFWD);
                         strcat(CandidatesFileName,"/");
                         strcat(CandidatesFileName, IPBNum);
                         strcat(CandidatesFileName, ".candidates");

                         if (LogLevel > LOG_INFO)
                         {
                              sprintf(LogBuffer, "Opening Candidates file <%s>", CandidatesFileName);
                              Log(LogTag, LogBuffer);
                         }

                         if (!(CandidatesFP = fopen(CandidatesFileName, "w")))
                         {
                              strcpy(LogBuffer, strerror(errno));
                              Log(LogTag, LogBuffer);
                              RCode--;
                         }
                         else if (LogLevel > LOG_INFO)
                         {
                              sprintf(LogBuffer, "Candidates file <%s> opened.", CandidatesFileName);
                              Log(LogTag, LogBuffer);
                         }
                    }

                    if (!RCode)
                    {
                         strcpy(RejectsFileName, ptrFWD);
                         strcat(RejectsFileName,"/");
                         strcat(RejectsFileName, IPBNum);
                         strcat(RejectsFileName, ".rejects");

                         if (LogLevel > LOG_INFO)
                         {
                              sprintf(LogBuffer, "Opening Rejects file <%s>", RejectsFileName);
                              Log(LogTag, LogBuffer);
                         }

                         if (!(RejectsFP = fopen(RejectsFileName, "w")))
                         {
                              strcpy(LogBuffer, strerror(errno));
                              Log(LogTag, LogBuffer);
                              RCode--;
                         }
                         else if (LogLevel > LOG_INFO)
                         {
                              sprintf(LogBuffer, "Rejects file <%s> opened.", RejectsFileName);
                              Log(LogTag, LogBuffer);
                         }
                    }

                    if (!RCode)
                    {
                         if (LogLevel > LOG_INFO)
                              Log(LogTag, "Writing IPB record to rejects file");

                         fputs(TmpBuffer, RejectsFP);
                         fflush(RejectsFP);
                    }
               }
               else if (!RecordDone && !strncmp(ptrRecord, "02", 2))
               {
                    if (LogLevel > LOG_INFO)
                    {
                         sprintf(LogBuffer, "Figure Record is: <%s>", ptrRecord);
                         Log(LogTag, LogBuffer);

                         Log(LogTag, "Writing Figure record to rejects file");
                    }


                    fputs(ptrRecord, RejectsFP);
                    fflush(RejectsFP);
               }
               else if (!RecordDone && !strncmp(ptrRecord, "03", 2))
               {
                    /*
                     * Find the XML Tag List in the Part record
                     */
                    if (!RecordDone)
                    {
                         if ((TagList = strstr(ptrRecord, BoundingTag)) == (char *)NULL)
                         {
                              if (LogLevel > LOG_INFO)
                                   Log(LogTag, "TagList not found.  Adding record to rejects file.");

                              fputs(ptrRecord, RejectsFP);
                              fflush(RejectsFP);
                              RecordDone++;
                         }
                         else if (LogLevel > LOG_INFO)
                         {
                              sprintf(LogBuffer, "TagList = <%s>", TagList);
                              Log(LogTag, LogBuffer);
                         }
                    }

                    /*
                     * Get Tag Status in Tag List
                     */
                    if (!RecordDone)
                    {
                         if (getTag(TSOpenTag, TSCloseTag, &TagStatus, TagList))
                         {
                              if (LogLevel > LOG_ERROR)
                              {
                                   Log(LogTag, "Error getting TagStatus.  Adding record to rejects file.");
                                   sprintf(LogBuffer, "Tags = <%s>", TagList);
                                   Log(LogTag, LogBuffer);
                              }

                              fputs(ptrRecord, RejectsFP);
                              fflush(RejectsFP);
                              RecordDone++;

                         }
                         else
                         {
                              if (LogLevel > LOG_INFO)
                              {
                                   sprintf(LogBuffer, "Tag Status = <%s>", TagStatus);
                                   Log(LogTag, LogBuffer);
                              }

                              if (strcmp(TagStatus, "Yes"))
                              {
                                   if (LogLevel > LOG_INFO)
                                   {
                                        Log(LogTag, "Tag Status not Yes.  Adding record to rejects file");
                                        sprintf(LogBuffer, "Tags = <%s>", TagList);
                                        Log(LogTag, LogBuffer);
                                   }

                                   fputs(ptrRecord, RejectsFP);
                                   fflush(RejectsFP);
                                   RecordDone++;
                              }
                         }
                    }

                    /*
                     * Get Drawing Number in Tag List
                     */
                    if (!RecordDone)
                    {
                         if (getTag(DwgNumOpenTag, DwgNumCloseTag, &DwgNum, TagList))
                         {
                              if (LogLevel > LOG_ERROR)
                              {
                                   Log(LogTag, "Error getting Drawing Number.  Adding record to rejects file.");
                                   sprintf(LogBuffer, "Tags = <%s>", TagList);
                                   Log(LogTag, LogBuffer);
                              }
                              fputs(ptrRecord, RejectsFP);
                              fflush(RejectsFP);
                              RecordDone++;

                         }
                         else
                         {
                              if (LogLevel > LOG_INFO)
                              {
                                   sprintf(LogBuffer, "Drawing Number = <%s>", DwgNum);
                                   Log(LogTag, LogBuffer);
                              }

                              if (!strlen(DwgNum))
                              {
                                   if (LogLevel > LOG_INFO)
                                   {
                                        Log(LogTag, "No Drawing Number.  Adding record to rejects file");
                                        sprintf(LogBuffer, "Tags = <%s>", TagList);
                                        Log(LogTag, LogBuffer);
                                   }

                                   fputs(ptrRecord, RejectsFP);
                                   fflush(RejectsFP);
                                   RecordDone++;
                              }
                         }
                    }

                    /*
                     * Get Drawing Number Cage
                     */
                    if (!RecordDone)
                    {
                         if (getTag(DNCOpenTag, DNCCloseTag, &DNCage, TagList))
                         {
                              if (LogLevel > LOG_ERROR)
                              {
                                   Log(LogTag, "Error getting Drawing Number Cage.  Adding record to rejects file.");
                                   sprintf(LogBuffer, "Tags = <%s>", TagList);
                                   Log(LogTag, LogBuffer);
                              }

                              fputs(ptrRecord, RejectsFP);
                              fflush(RejectsFP);
                              RecordDone++;

                         }
                         else
                         {
                              if (LogLevel > LOG_INFO)
                              {
                                   sprintf(LogBuffer, "Drawing Number Cage = <%s>", DNCage);
                                   Log(LogTag, LogBuffer);
                              }

                              /*
                               * If Drawing Number Cage is not 5 characters long
                               */
                              if (strlen(DNCage) != 5)
                              {
                                   if (LogLevel > LOG_ERROR)
                                   {
                                        Log(LogTag, "Drawing Number Cage is not 5 characters long.  Adding record to rejects file");
                                        sprintf(LogBuffer, "Tags = <%s>", TagList);
                                        Log(LogTag, LogBuffer);
                                   }

                                   fputs(ptrRecord, RejectsFP);
                                   fflush(RejectsFP);
                                   RecordDone++;
                              }
                         }
                    }

                    /*
                     * Get Part Number
                     */
                    if (!RecordDone)
                    {
                         if (getTag(PartNumOpenTag, PartNumCloseTag, &PartNum, TagList))
                         {
                              if (LogLevel > LOG_ERROR)
                              {
                                   Log(LogTag, "Error getting Part Number. Adding record to rejects file.");
                                   sprintf(LogBuffer, "Tags = <%s>", TagList);
                                   Log(LogTag, LogBuffer);
                              }

                              fputs(ptrRecord, RejectsFP);
                              fflush(RejectsFP);
                              RecordDone++;
                         }
                         else
                         {
                              if (LogLevel > LOG_INFO)
                              {
                                   sprintf(LogBuffer, "Part Number = <%s>", PartNum);
                                   Log(LogTag, LogBuffer);
                              }

                              if (!strlen(PartNum))
                              {
                                   if (LogLevel > LOG_ERROR)
                                   {
                                        Log(LogTag, "No Part Number.  Adding record to rejects file");
                                        sprintf(LogBuffer, "Tags = <%s>", TagList);
                                        Log(LogTag, LogBuffer);
                                   }

                                   fputs(ptrRecord, RejectsFP);
                                   fflush(RejectsFP);
                                   RecordDone++;
                              }
                         }
                    }

                    /*
                     * Get Part Number Cage
                     */
                    if (!RecordDone)
                    {
                         if (getTag(PNCOpenTag, PNCCloseTag, &PNCage, TagList))
                         {
                              if (LogLevel > LOG_ERROR)
                              {
                                   Log(LogTag, "Error getting Part Number Cage");
                                   sprintf(LogBuffer, "Tags = <%s>", TagList);
                                   Log(LogTag, LogBuffer);
                              }

                              fputs(ptrRecord, RejectsFP);
                              fflush(RejectsFP);
                              RecordDone++;
                         }
                         else
                         {
                              if (LogLevel > LOG_INFO)
                              {
                                   sprintf(LogBuffer, "Part Number Cage = <%s>", PNCage);
                                   Log(LogTag, LogBuffer);
                              }

                              /*
                               * If Part Number Cage is not 5 characters long
                               */
                              if (strlen(PNCage) != 5)
                              {
                                   if (LogLevel > LOG_ERROR)
                                   {
                                        Log(LogTag, "Part Number Cage not 5 characters long.  Adding record to rejects file");
                                        sprintf(LogBuffer, "Tags = <%s>", TagList);
                                        Log(LogTag, LogBuffer);
                                   }

                                   fputs(ptrRecord, RejectsFP);
                                   fflush(RejectsFP);
                                   RecordDone++;
                              }
                         }
                    }

                    /*
                     * Get Part Type
                     */
                    if (!RecordDone)
                    {
                         if (getTag(PTOpenTag, PTCloseTag, &PartType, TagList))
                         {
                              if (LogLevel > LOG_ERROR)
                              {
                                   Log(LogTag, "Error getting Part Type");
                                   sprintf(LogBuffer, "Tags = <%s>", TagList);
                                   Log(LogTag, LogBuffer);
                              }

                              fputs(ptrRecord, RejectsFP);
                              fflush(RejectsFP);
                              RecordDone++;
                         }
                         else
                         {
                              if (LogLevel > LOG_INFO)
                              {
                                   sprintf(LogBuffer, "Part Type = <%s>", PartType);
                                   Log(LogTag, LogBuffer);
                              }

                              if (!strlen(PartType))
                              {
                                   if (LogLevel > LOG_ERROR)
                                   {
                                        Log(LogTag, "No Part Type.  Adding record to rejects file");
                                        sprintf(LogBuffer, "Tags = <%s>", TagList);
                                        Log(LogTag, LogBuffer);
                                   }

                                   fputs(ptrRecord, RejectsFP);
                                   fflush(RejectsFP);
                                   RecordDone++;
                              }
                         }
                    }

                    if (!RecordDone)
                    {
                         if (LogLevel > LOG_INFO)
                              Log(LogTag, "Adding record to candidates file and SQL script");
                         fputs(ptrRecord, CandidatesFP);
                         fflush(CandidatesFP);
                         writeSQLStatement(SQLFP, DwgNum, DNCage, PartNum, PNCage, PartType, LogLevel);
                         fflush(SQLFP);

                    }
               }
               else if (!RecordDone)
               {
                    if (LogLevel > LOG_INFO)
                    {
                         sprintf(LogBuffer, "Invalid record type: <%s>", ptrRecord);
                         Log(LogTag, LogBuffer);

                         Log(LogTag, "Writing record to rejects file");
                    }

                    fputs(ptrRecord, RejectsFP);
                    fflush(RejectsFP);

                    RecordDone++;

               }

               if (ptrRecord)
                    free(ptrRecord);

               if (DwgNum)
                    free(DwgNum);

               if (DNCage)
                    free(DNCage);

               if (PartNum)
                    free(PartNum);

               if (PNCage)
                    free(PNCage);

               if (PartType)
                    free(PartType);
          }

          if (SQLFP)
          {
               fprintf(SQLFP, "Exit;\n");
               fflush(SQLFP);
               fclose(SQLFP);
          }

          if (CandidatesFP)
               fclose(CandidatesFP);

          if (RejectsFP)
               fclose(RejectsFP);

          if (InputFP)
               fclose (InputFP);

          if (LogLevel > LOG_WARN)
          {
               sprintf(LogBuffer, "Finished processing <%s>", FileName);
               Log(LogTag, LogBuffer);
          }
     }
     else if (LogLevel > LOG_OFF)
     {
          strcpy(LogBuffer, strerror(errno));
          Log(LogTag, LogBuffer);
          RCode--;
     }

     if (LFPtr)
          fclose(LFPtr);

     return RCode;
}

int readLine (FILE *fp, char **Buffer)
{
     int cur_max = MAX_BUFFER_SIZE;
     int  count = 0;
     int length = 0;
     char ch = (char)NULL;
     int RCode = NO_ERROR;
     char LogBuffer[LOG_BUFFER_LEN];
     char *LogTag = "readLine";

     if (!((char *)*Buffer = (char *)malloc(sizeof(char) * cur_max)))
     {
          RCode--;
          if (LogLevel > LOG_OFF)
          {
               sprintf(LogBuffer, "Error allocating %d bytes for Buffer", sizeof(char) * cur_max);
               Log(LogTag, LogBuffer);
          }
     }
     else if (LogLevel > LOG_INFO)
     {
          sprintf(LogBuffer, "Allocated %d bytes for Buffer", sizeof(char) * cur_max);
          Log(LogTag, LogBuffer);
     }
   
     while (!RCode && (ch != '\n') && (ch != EOF))
     {
          if (count == cur_max)
          {
               cur_max *= 2;
               count = 0;

               if (!((char *)*Buffer = (char *)realloc((char *)*Buffer, cur_max)))
               {
                    RCode--;
                    if (LogLevel > LOG_OFF)
                    {
                         sprintf(LogBuffer, "Error reallocating %d additional bytes for Buffer", cur_max);
                         Log(LogTag, LogBuffer);
                    }
               }
               else if (LogLevel > LOG_INFO)
               {
                    sprintf(LogBuffer, "Reallocated %d additional bytes for Buffer", cur_max);
                    Log(LogTag, LogBuffer);
               }
          }
          ch = fgetc(fp);
          (*Buffer)[length] = ch;
          length++;
          count++;
     }

     (*Buffer)[length] = (char)NULL;

     return RCode;

}

int getTag(char *OpenTagPtr, char *CloseTagPtr, char **Buffer, char *TagListPtr)
{
     int RCode = NO_ERROR;
     int length = 0;
     char *BeginPtr, *EndPtr;
     char LogBuffer[LOG_BUFFER_LEN];
     char *LogTag = "getTag";


     /*
      * Fing the open tag
      */
     if ((BeginPtr = strstr(TagListPtr, OpenTagPtr)) != (char *)NULL)
     {

          if (LogLevel > LOG_INFO)
          {
              sprintf(LogBuffer, "Found tag %s", OpenTagPtr);
              Log(LogTag, LogBuffer);
          }

          /*
           * Jump to beginning of tag content
           */
          BeginPtr = BeginPtr + strlen(OpenTagPtr);

          if (LogLevel > LOG_INFO)
          {
              sprintf(LogBuffer, "Beginning of content = %s", BeginPtr);
              Log(LogTag, LogBuffer);
          }

          /*
           * Find close tag
           */
          if ((EndPtr = strstr(BeginPtr, CloseTagPtr)) != (char *)NULL)
          {

               /*
                * Determine length of content + 1 for NULL
                */
               length = EndPtr - BeginPtr + 1;

               if (LogLevel > LOG_INFO)
               {
                   sprintf(LogBuffer, "Length of content + NULL = %d", length);
                   Log(LogTag, LogBuffer);
               }

               /*
                * Allocate space for content
                */
               if (((char *)*Buffer = (char *)malloc(sizeof(char) * length)) != (char *)NULL)
               {

                    /*
                     * Copy content
                     */
                    strlcpy((char *)*Buffer, BeginPtr, length);

                    if (LogLevel > LOG_INFO)
                    {
                        sprintf(LogBuffer, "Copied <%s> to buffer", *Buffer);
                        Log(LogTag, LogBuffer);
                    }

               }
               else if (LogLevel > LOG_OFF)
               {
                    Log(LogTag, "Error allocating space for buffer");
                    RCode--;
               }
          }
          else if (LogLevel > LOG_OFF)
          {
               sprintf(LogBuffer, "Error: Did not find close tag <%s>", CloseTagPtr);
               Log(LogTag, LogBuffer);
               RCode--;
          }
     }
     else if (LogLevel > LOG_OFF)
     {
          sprintf(LogBuffer, "Did not find open tag <%s>", OpenTagPtr);
          Log(LogTag, LogBuffer);
          RCode--;
     }

     return RCode;

}


int writeSQLStatement(FILE *FP, char *DwgNumPtr, char *DwgCagePtr, char *PartNumPtr, char *PartCagePtr, char *PartTypePtr, int LL)
{
    
     fprintf(FP, "EXECUTE ADD_PART('%s', '%s', '%s', '%s', '%s', '%s', %d);\n", IPBNum, PartNumPtr, PartCagePtr, PartTypePtr, DwgNumPtr, DwgCagePtr, LL);
     fflush(FP);

     return 0;
}

int Log(const char *Tag, const char *Message)
{
     time_t sysTime; 
     struct tm localTime;
     char TimeBuffer[TIME_BUFFER_LEN];

     time(&sysTime);

     localtime_r(&sysTime, &localTime);

     strftime(TimeBuffer, sizeof(TimeBuffer), "%Y-%m-%d %H:%M:%S", &localTime);
    
     if (!LFPtr)
     {
          if ((LFPtr = fopen(LogFileName, "a")))
          {
               if (LogLevel > LOG_INFO)
               {
                    fprintf(LFPtr, "%s (N/A) [%s]: log file <%s> opened\n", TimeBuffer, "Log", LogFileName);
                    fflush(LFPtr);
               }
          }
     }


     if (LFPtr)
     {
         fprintf(LFPtr, "%s (%s) [%s]: %s\n", TimeBuffer, strlen(IPBNum) ? IPBNum : "N/A", Tag, Message);
         fflush(LFPtr);
     }

}

convertToUpperCase(char *Ptr)
{
     while (*Ptr != NULL)
     {
          *Ptr = toupper((unsigned char)*Ptr);
          Ptr++;
     }
}
