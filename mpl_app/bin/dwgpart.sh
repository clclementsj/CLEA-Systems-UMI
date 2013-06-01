#!/bin/bash

if [[ ! $MPL_CONFIG ]]; then
     echo MPL_CONFIG not defined!
     exit
elif [ ! -f $MPL_CONFIG ]; then
     echo MPL_CONFIG '('$MPL_CONFIG')' does not exist!
     exit
fi

. $MPL_CONFIG

#
# Verify that all environment variables are set
#

if [[ ! $MPL_WORK_DIR ]]; then
     echo MPL_WORK_DIR not defined.
     exit
elif [ ! -d $MPL_WORK_DIR ]; then
     echo MPL_WORK_DIR '('$MPL_WORK_DIR')' does not exist!
     exit
fi

if [[ ! $MPL_APP_DIR ]]; then
     echo MPL_APP_DIR not defined! >> $MPL_WORK_DIR/dwgpart.log
     exit
elif [ ! -d $MPL_APP_DIR ]; then
     echo MPL_APP_DIR '('$MPL_APP_DIR')' does not exist! >> $MPL_WORK_DIR/dwgpart.log
     exit
fi

if [[ ! $MPL_FILES_DIR ]]; then
     echo MPL_FILES_DIR not defined. >> $MPL_WORK_DIR/dwgpart.log
     exit
elif [[ ! -d $MPL_FILES_DIR ]]; then
     echo MPL_FILES_DIR '('$MPL_FILES_DIR')' does not exist! >> $MPL_WORK_DIR/dwgpart.log
     exit
fi

if [[ ! $MPL_STOP_FILE ]]; then
     echo MPL_STOP_FILE not defined. >> $MPL_WORK_DIR/dwgpart.log
     exit
fi

if [[ ! $MPL_LOG_LEVEL ]]; then
     echo MPL_LOG_LEVEL not defined. >> $MPL_WORK_DIR/dwgpart.log
     exit
fi


#
# Loop through MPL files
#
if [[ $(ls -A $MPL_FILES_DIR) ]]; then
     #
     # Create stored procedure and report table
     #
     if [ -f $MPL_APP_DIR/pre_process.sql ]; then
          sqlplus -s / as sysdba @$MPL_APP_DIR/pre_process.sql >> $MPL_WORK_DIR/dwgpart.log 2>&1
      else
          LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
          echo $LOG_DATE '('N/A')' [dwgpart.sh] $MPL_APP_DIR/pre_process.sql not found.  Exiting. >> $MPL_WORK_DIR/dwgpart.log 2>&1
          exit
     fi

     for file in $(ls $MPL_FILES_DIR);
     do

          #
          # If stop file does not exist, process the file
          #
          if [ ! -f $MPL_STOP_FILE ]; then

               #
               # Create IPB-based working directory
               #
               ipb=${file%%_*}

               IPB=`echo $ipb | tr '[:lower:]' '[:upper:]'`

               DATE=$(date +"%Y%m%d")
 
               FILE_WORK_DIR=$MPL_WORK_DIR/$DATE-$IPB

               export FILE_WORK_DIR

               mkdir -p $FILE_WORK_DIR

               if [ $? -ne 0 ]; then
                   LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                   echo $LOG_DATE '('$IPB')' [dwgpart.sh] Error creating $FILE_WORK_DIR.  Processing stopped. >> $MPL_WORK_DIR/dwgpart.log
                   break
               fi
                   

               #
               # Move MPL file to IPB work directory
               #
               mv $MPL_FILES_DIR/$file $FILE_WORK_DIR
 
               #
               # Process MPL file
               #
               $MPL_APP_DIR/dwgpart -f $FILE_WORK_DIR/$file -L $MPL_LOG_LEVEL -l $MPL_WORK_DIR/dwgpart.log >> $MPL_WORK_DIR/dwgpart.log 2>&1

               if [ $? -eq 0 ]; then
                    if [ $MPL_LOG_LEVEL -gt 2 ]; then
                         LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                         echo $LOG_DATE '('$IPB')' [dwgpart.sh] Executing $FILE_WORK_DIR/$IPB.sql >> $MPL_WORK_DIR/dwgpart.log
                    fi
 
                    #
                    # Execute generated SQL script
                    #

                    if [ -f $FILE_WORK_DIR/$IPB.sql ]; then
                         sqlplus -s /  as sysdba @$FILE_WORK_DIR/$IPB.sql >> $MPL_WORK_DIR/dwgpart.log 2>&1
                    else
                         LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                         echo $LOG_DATE '('$IPB')' [dwgpart.sh] $FILE_WORK_DIR/$IPB.sql not found. >> $MPL_WORK_DIR/dwgpart.log
                    fi

                    if [ $MPL_LOG_LEVEL -gt 2 ]; then

                         LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                         echo $LOG_DATE '('$IPB')' [dwgpart.sh] Finished executing $FILE_WORK_DIR/$IPB.sql >> $MPL_WORK_DIR/dwgpart.log

                         LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                         echo $LOG_DATE '('$IPB')' [dwgpart.sh] Executing $MPL_APP_DIR/dwg_part_rpt.sql >> $MPL_WORK_DIR/dwgpart.log

                    fi

                    #
                    # Generate report
                    #
                    if [ -f $MPL_APP_DIR/dwg_part_rpt.sql ]; then
                         sqlplus -s /  as sysdba @$MPL_APP_DIR/dwg_part_rpt.sql > $FILE_WORK_DIR/$IPB.rpt 2>&1
                    else
                         LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                         echo $LOG_DATE '('$IPB')' [dwgpart.sh] $MPL_APP_DIR/dwg_part_rpt.sql not found. >> $MPL_WORK_DIR/dwgpart.log
                    fi


                    if [ $MPL_LOG_LEVEL -gt 2 ]; then
                         LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                         echo $LOG_DATE '('$IPB')' [dwgpart.sh] Finished executing $MPL_APP_DIR/dwg_part_rpt.sql >> $MPL_WORK_DIR/dwgpart.log
                    fi

               else
                   LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
                   echo $LOG_DATE '('$IPB')' [dwgpart.sh] Error processing $FILE_WORK_DIR/$file. >> $MPL_WORK_DIR/dwgpart.log
               fi
                   
          #
          # Stop file found.  Stop processing files
          #
          else

               LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
               echo $LOG_DATE '('N/A')' [dwgpart.sh] $MPL_STOP_FILE found.  Process stopped. >> $MPL_WORK_DIR/dwgpart.log

               break

          fi
 
     done

     #
     # Drop the stored procedure and dwg_part_rpt table.
     #
     if [ -f $MPL_APP_DIR/post_process.sql ]; then
          sqlplus -s /  as sysdba @$MPL_APP_DIR/post_process.sql >> $MPL_WORK_DIR/dwgpart.log
     else
          LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
          echo $LOG_DATE '('N/A')' [dwgpart.sh] $MPL_APP_DIR/post_process.sql not found. >> $MPL_WORK_DIR/dwgpart.log
     fi

else
     LOG_DATE=$(date +"%Y-%m-%d %H:%M:%S")
     echo $LOG_DATE '('N/A')' [dwgpart.sh] No files to process.  Exiting. >> $MPL_WORK_DIR/dwgpart.log
fi


