prompt
prompt *** Part Numbers associated with Drawing Revisions. *** 

set feedback on
set pagesize 49999
set linesize 120

col dwg_num heading "Dwg Number"
col dwg_cage format a9 heading "Dwg CAGE"
col dwg_rev heading "Dwg Rev"
col part_num heading "Part Number"
col part_cage format a9 heading "Part CAGE"

break on part_num

set feedback off
select to_char(sysdate, 'DD-MON-YYYY:HH:MI:SS') "Report Date" from dual;
set feedback on

select part_num, part_cage, dwg_num, dwg_cage, dwg_rev from dwg_part_rpt
order by part_num, part_cage, dwg_num, dwg_cage, dwg_rev;

exit;

/

