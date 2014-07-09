/* simplepb.h

   Copyright (C) 2005, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 08 December 2005
   Last Change: Tuesday 17 January 2006
   Last Commit: $Date: 2006/03/13 18:10:43 $ (UTC)
   Last Changed by: $Author: jon $

*/

#ifndef simplepb_h
#define simplepb_h
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <oleauto.h>


extern "C"
{
   ////////////////////////////////////////////////////////////
   // OpenPort
   //
   // Specifies that the specified com port number should be used.  Returns 0
   // on success or -1 on failure. 
   ////////////////////////////////////////////////////////////
   int __stdcall OpenPort(
      int com_port_no,
      int baud);


   ////////////////////////////////////////////////////////////
   // OpenIPPort
   //
   // Specifies that a TCP connection should be used.  Returns 0 on success or
   // -1 on failure. 
   ////////////////////////////////////////////////////////////
   int __stdcall OpenIPPort(
      char const *ip_address,
      int tcp_port);


   ////////////////////////////////////////////////////////////
   // ClosePort
   //
   // Closes a connection made previously with OpenPort or OpenIPPort.  The
   // return value should always be zero.
   ////////////////////////////////////////////////////////////
   int __stdcall ClosePort();


   ////////////////////////////////////////////////////////////
   // CloseIPPort
   //
   // Synonymous with ClosePort().
   ////////////////////////////////////////////////////////////
   int __stdcall CloseIPPort();

   ////////////////////////////////////////////////////////////
   // GetLastResults
   //
   // Returns the text results of the last call made to the rest of the
   // functions defined in this header.  This function is meant to simplify the
   // interface for visual basic programs.
   ////////////////////////////////////////////////////////////
   BSTR __stdcall GetLastResults();

   ////////////////////////////////////////////////////////////
   // GetClock
   //
   // Attempts to read the clock from the specified device type and address.
   // If the clock is read, the datalogger time will be returned in the pointer
   // set by return_data.  The return value will match one of those declared in
   // enum clock_return. 
   ////////////////////////////////////////////////////////////
   enum device_type_codes
   {
      device_cr200 = 1,
      device_cr10xpb = 2,
      device_cr1000 = 3,
      device_cr3000 = 4,
      device_cr800 = 5,
      device_cr5000 = 6,
      device_cr9000 = 7,        /* 9031 based BMP4 CR9000 using BMP4 */
      device_cr9000x = 8,       /* 9032 based CR9000 using BMP4*/
   };
   enum clock_return
   {
      clock_success = 0,
      clock_comm_failure = -1,
      clock_port_not_opened = -2
   };
   int __stdcall GetClock(
      int pakbus_address,
      int device_type,
      char **return_data,
      int *return_data_len);


   ////////////////////////////////////////////////////////////
   // SetClock
   //
   // Attempts to set the clock for the specified device type and address.  If
   // the clock was set, the old time and new time will be returned in the
   // pointer set by return_data.  The return value will match one of the
   // values defined in the clock_return enum.
   ////////////////////////////////////////////////////////////
   int __stdcall SetClock(
      int pakbus_address,
      int device_type,
      char **return_data,
      int *return_data_len);


   ////////////////////////////////////////////////////////////
   // GetValue
   //
   // Attempts to retrieve values from the specified device type and address.
   // If the transaction succeeds, the values will be written to the return
   // data as a comma separated list of values (one value for each scalar
   // requested).  The return value will correspond with one of the values in
   // getval_return. 
   ////////////////////////////////////////////////////////////
   enum getval_return
   {
      getval_success = 0,
      getval_comm_failure = -1,
      getval_port_not_opened = -2
   };
   int __stdcall GetValue(
      int pakbus_address,
      int device_type,
      int swath,
      char const *table_name,
      char const *field_name,
      char **return_data,
      int *return_data_len);


   ////////////////////////////////////////////////////////////
   // SetValue
   //
   // Attempts to set a value in the datalogger.  The return value will
   // correspond with one of the setval_return enumerations.
   ////////////////////////////////////////////////////////////
   enum setval_return
   {
      setval_success = 0,
      setval_comm_failure = -1,
      setval_not_opened = -2
   };
   int __stdcall SetValue(
      int pakbus_address,
      int device_type,
      char const *table_name,
      char const *field_name,
      char const *value);


   ////////////////////////////////////////////////////////////
   // GetData
   //
   // Starts or continues a query for datalogger records.  The table number
   // specifies the datalogger table number.  The record number can be one of
   // the following:
   //
   //   -1 : collect only the most recent record
   //    0 : collect oldest to newest records
   //  num : collect starting at num up to newest (starts at oldest if num is
   //        no longer in the table).
   //
   // The return data will be formatted according to the following syntax:
   //
   //   ret-data := "\"" time-stamp "\","  record-no "\r\n"
   //               { field-no "," field-name "," field-val "\r\n" }
   //
   // The return value will match one of the values defined in getdata_return.
   // If the return value is one, the application can call this function again
   // with the same parameters to get the next record.  This repetition can
   // continue until the return value is zero.  
   ////////////////////////////////////////////////////////////
   enum getdata_return
   {
      getdata_more = 1,
      getdata_complete = 0,
      getdata_comm_failure = -1,
      getdata_not_open = -2,
      getdata_invalid_table_no = -3
   };
   int __stdcall GetData(
      int pakbus_address,
      int device_type,
      int table_no,
      int record_no,
      char **return_data,
      int *return_data_len);


   ////////////////////////////////////////////////////////////
   // GetDataHeader
   //
   // Returns the TOA5 header for the specified table number in the
   // return_data.  The return value will correspond with one of the values
   // defined by getdata_return.
   ////////////////////////////////////////////////////////////
   int __stdcall GetDataHeader(
      int pakbus_address,
      int device_type,
      int table_no,
      char **return_data,
      int *return_len);


   ////////////////////////////////////////////////////////////
   // GetCommaData
   //
   // Queries the datalogger for records and returns each record as a TOA5
   // comma separated record.  The return value will correspond with one of the
   // values defined in getdata_return.
   ////////////////////////////////////////////////////////////
   int __stdcall GetCommaData(
      int pakbus_address,
      int device_type,
      int table_no,
      int record_no,
      char **return_data,
      int *return_len);


   ////////////////////////////////////////////////////////////
   // FileSend
   //
   // Sends the specified datalogger program to the datalogger.  The function
   // will return with a value of one as the operation progresses.  In each of
   // these returns, the return_data will contain a string that indicates the
   // progress.  The application can continue the operation by invoking the
   // same function with the same parameters.  Once the operation is complete,
   // the return data will contain the datalogger compile results and the
   // return value will be zero.  The return value will match one of the values
   // defined in filesend_return.
   ////////////////////////////////////////////////////////////
   enum filesend_return
   {
      filesend_more = 1,
      filesend_complete = 0,
      filesend_comm_failure = -1,
      filesend_not_opened = -2,
      filesend_cant_open_source = -3,
      filesend_file_name_invalid = -4,
      filesend_logger_timeout = -5,
      filesend_invalid_file_offset = -6,
      filesend_datalogger_error = -7,
      filesend_filecontrol_error = -8,
      filesend_cant_get_prog_status = -9
   };
   int __stdcall File_Send(
      int pakbus_address,
      int device_type,
      char const *file_name,
      char **return_data,
      int *return_len);


   ////////////////////////////////////////////////////////////
   // GetAddress
   //
   // Queries the port for a PakBus device.  If a device is found, the address
   // of that device will be formatted in the return_data as
   // "PakBusAddress=x;".  The return value will correspond with one of the
   // values defined in getaddr_return
   ////////////////////////////////////////////////////////////
   enum getaddr_return
   {
      getaddr_success = 0,
      getaddr_comm_failure = -1,
      getaddr_not_open = -2
   };
   int __stdcall GetAddress(
      int device_type,
      char  **return_data,
      int *return_len);


   ////////////////////////////////////////////////////////////
   // GetStatus
   //
   // Queries the logger status table and returns the status data formatted
   // using the following syntax:
   //
   //   return-data := { field-name ": " field-value "\r\n" }.
   //
   // The return value will correspond with one of the values defined in
   // getstatus_return. 
   ////////////////////////////////////////////////////////////
   enum getstatus_return
   {
      getstatus_success = 0,
      getstatus_comm_failure = -1,
      getstatus_not_open = -2,
   };
   int __stdcall GetStatus(
      int pakbus_address,
      int device_type,
      char **return_data,
      int *return_data_len);


   ////////////////////////////////////////////////////////////
   // GetTableNames
   //
   // Queries the datalogger for table definitions and returns the list of
   // table names and numbers in the return_data parameter using the following
   // syntax:
   //
   //   return-data := { table-no " " table-name "\r\n" }.
   //
   // The return value will correspond with one of the values in
   // gettables_return. 
   ////////////////////////////////////////////////////////////
   enum gettables_return
   {
      gettables_success = 0,
      gettables_comm_failure = 1,
      gettables_not_open = -2
   };
   int __stdcall GetTableNames(
      int pakbus_address,
      int device_type,
      char **return_data,
      int *return_data_len);


   ////////////////////////////////////////////////////////////
   // GetDllVersion
   //
   // Returns the version info for this DLL
   ////////////////////////////////////////////////////////////
   int __stdcall GetDLLVersion(
      char  **return_data,
      int *return_data_len);

   
   ////////////////////////////////////////////////////////////
   // FileControl
   //
   // Performs the specified file control action on the logger.  The action
   // taken will depend on the file name and command arguments.  The command
   // should be one of the values defined in file_control_command_type   The
   // return value will correspond with one of the values defined in
   // filecontrol_return. 
   ////////////////////////////////////////////////////////////
   enum file_control_command_type
   {
      command_compile_and_run = 1,
      command_set_run_on_power_up = 2,
      command_make_hidden = 3,
      command_delete_file = 4,
      command_format_device = 5,
      command_compile_and_run_leave_tables = 6,
      command_stop_program = 7,
      command_stop_program_and_delete = 8,
      command_make_os = 9,
      command_compile_and_run_no_power_up = 10,
      command_pause = 11,
      command_resume = 12,
      command_stop_delete_and_run = 13,
      command_stop_delete_and_run_no_power = 14
   };
   enum filecontrol_return
   {
      filecontrol_success = 0,
      filecontrol_comm_failure = -1,
      filecontrol_not_open = -2
   };
   int __stdcall FileControl(
      int pakbus_address,
      int device_type,
      char const *file_name,
      int command);
};


#endif
