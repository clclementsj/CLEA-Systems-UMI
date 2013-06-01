set termout off

CREATE TABLE DWG_PART_RPT(
     DWG_NUM      VARCHAR2(32),
     DWG_CAGE     VARCHAR2(5),
     DWG_REV      VARCHAR2(8),
     PART_NUM     VARCHAR2(32),
     PART_CAGE    VARCHAR2(5));

CREATE TABLE EDMICS.DWG_PART_RPT_HIST(
     DWG_NUM      VARCHAR2(32),
     DWG_CAGE     VARCHAR2(5),
     DWG_REV      VARCHAR2(8),
     PART_NUM     VARCHAR2(32),
     PART_CAGE    VARCHAR2(5));


CREATE PROCEDURE add_part(ipb varchar2, pn varchar2, pnc varchar2, pt varchar2, dn varchar2, dnc varchar2, loglevel INTEGER) IS
     p_exists INTEGER;
     x_exists INTEGER;
     p_sys_id EDMICS.PART.PART_SYS_ID%TYPE;
     v_err_code NUMBER;
     v_err_msg VARCHAR2(512);
     CURSOR dwg_rev_cur(dnum varchar2, dn_cage varchar2) IS 
            SELECT * FROM edmics.dwg_revisions
            WHERE dwg_num = dnum and cage = dn_cage;
BEGIN

     DBMS_OUTPUT.ENABLE;

     -- Determine whether the PARTS record already exists
     IF (loglevel > 3) THEN

          DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] Checking whether the PART record exists with PART_NUM = ' || pn || ' and PART_CAGE = ' || pnc);

     END IF;

     SELECT COUNT(*) INTO p_exists
     FROM EDMICS.PART
     WHERE part_num = pn AND part_cage = pnc
     AND ROWNUM = 1;

     -- If the PART record already exists, get it's PART_SYS_ID
     IF (p_exists = 1) THEN

          IF (loglevel > 3) THEN

               DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] PART record already exists.  Getting the PART_SYS_ID');

          END IF;

          SELECT part_sys_id INTO p_sys_id
          FROM EDMICS.PART
          WHERE part_num = pn AND part_cage = pnc;

          IF (loglevel > 3) THEN

               DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] PART_SYS_ID = ' || p_sys_id);

          END IF;

     -- If the PART record does not already exist, get next sequence value
     -- and create PART record
     ELSE

          IF (loglevel > 3) THEN

               DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] PART record does not exist.  Getting the next PART_SYS_ID sequence value.');

          END IF;

          SELECT edmics.part_sys_id.nextval 
          INTO p_sys_id
          FROM DUAL;

          IF (loglevel > 3) THEN

               DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] Inserting PART record with PART_NUM = ' || pn || ', PART_CAGE = ' || pnc || ', PART_TYPE = ' || pt ||', and PART_SYS_ID = '|| p_sys_id);

          END IF;

          INSERT INTO edmics.part (part_num, part_cage, part_type, part_sys_id)
          VALUES (pn, pnc, pt, p_sys_id);

     END IF;
     
     -- Loop through DWG_REVISIONS records and create XREF_ASSNS records to
     -- associate PART record with DWG_REVISIONS records
     FOR dwg_rev IN dwg_rev_cur(dn, dnc) LOOP

          -- Determine whether the PARTS record already exists
          IF (loglevel > 3) THEN

               DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] Checking on whether XREF_ASSNS record exists with FROM_SYS_ID = ' || p_sys_id || ' AND TO_SYS_ID = ' || dwg_rev.dwg_rev_sys_id);

          END IF;

          SELECT COUNT(*) INTO x_exists
          FROM edmics.xref_assns
          WHERE from_sys_id = p_sys_id AND to_sys_id = dwg_rev.dwg_rev_sys_id
          AND ROWNUM = 1;


          -- If the XREF_ASSNS record does not exist, insert it
          IF (x_exists = 0) THEN

               IF (loglevel > 3) THEN

                    DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] XREF_ASSNS record does not exist.  Inserting record with FROM_SYS_ID = ' || p_sys_id || ', TO_SYS_ID = ' || dwg_rev.dwg_rev_sys_id || ', ASSN_CODE = PARTDWG, FROM_TYPE = PART, and TO_TYPE = DWGREV');

               END IF;


               INSERT INTO edmics.xref_assns (from_sys_id, to_sys_id, assn_code, from_type, to_type) 
               VALUES (p_sys_id, dwg_rev.dwg_rev_sys_id, 'PARTDWG', 'PART', 'DWGREV');

               IF (loglevel > 3) THEN

                    DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part]  Inserting DWG_PART_RPT record with DWG_NUM = ' || dn || ', DWG_CAGE = ' || dnc || ', DWG_REV = ' || dwg_rev.dwg_rev || ', PART_NUM = ' || pn || ', and PART_CAGE = ' || pnc);

               END IF;


               INSERT INTO DWG_PART_RPT (DWG_NUM, DWG_CAGE, DWG_REV, PART_NUM, PART_CAGE)
               VALUES (dn, dnc, dwg_rev.dwg_rev, pn, pnc);

          ELSE
               IF (loglevel > 3) THEN

                    DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] XREF_ASSNS record exists.  Doing nothing.');

               END IF;


          END IF;

     END LOOP;

     COMMIT;

EXCEPTION

     WHEN OTHERS THEN

          v_err_code := SQLCODE;
          v_err_msg := SQLERRM;
 
          DBMS_OUTPUT.PUT_LINE(to_char(sysdate, 'YYYY-MM-DD HH:MM:SS') || ' (' || ipb || ') [add_part] Exception: ' || to_char(v_err_code) || ' - ' || v_err_msg);

          ROLLBACK;

END ADD_PART;

/


exit;

/

