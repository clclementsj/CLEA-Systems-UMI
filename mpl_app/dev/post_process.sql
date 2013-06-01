set termout off

drop procedure add_part;

insert into edmics.dwg_part_rpt_hist select * from dwg_part_rpt;

drop table dwg_part_rpt;

exit;

/

