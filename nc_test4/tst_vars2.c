/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
    See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables.
   Ed Hartnett
*/

#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"

#define FILE_NAME "tst_vars2.nc"
#define NUM_DIMS 1
#define DIM1_LEN NC_UNLIMITED
#define DIM1_NAME "Hoplites_Engaged"
#define VAR_NAME "Battle_of_Marathon"
#define LOSSES_NAME "Miltiades_Losses"

#define MAX_CNUM 4

int
main(int argc, char **argv)
{
   int ncid, dimids[NUM_DIMS];
   int varid;
   int nvars_in, varids_in[NUM_DIMS];
   signed char fill_value = 42, fill_value_in;
   nc_type xtype_in;
   size_t len_in;
   char name_in[NC_MAX_NAME + 1];
   int attnum_in;
   int cnum;

   printf("\n*** Testing netcdf-4 variable functions, even more.\n");
   for (cnum = 0; cnum < MAX_CNUM; cnum++)
   {
      int cmode = 0;

      switch(cnum)
      {
         case 0:
            printf("*** Testing with classic format:\n");
            cmode = 0;
            break;
         case 1:
            printf("*** Testing with 64-bit offset format:\n");
            cmode = NC_64BIT_OFFSET;
            break;
         case 2:
            printf("*** Testing with HDF5:\n");
            cmode = NC_NETCDF4|NC_CLOBBER;
            break;
         case 3:
            printf("*** Testing with HDF5, netCDF Classic Model:\n");
            cmode = NC_CLASSIC_MODEL | NC_NETCDF4;
	    break;
	 default:
	    return 1;
      }

      printf("**** testing simple fill value attribute creation...");
      {
         int status;
         /* Create a netcdf-4 file with one scalar var. Add fill
          * value. */
         if (nc_create(FILE_NAME, cmode, &ncid)) ERR;
         if (nc_def_var(ncid, VAR_NAME, NC_BYTE, 0, NULL, &varid)) ERR;
         if (nc_put_att_schar(ncid, varid, _FillValue, NC_BYTE, 1, &fill_value)) ERR;
         if (nc_enddef(ncid)) ERR;
         if (nc_redef(ncid)) ERR;
         status = nc_put_att_schar(ncid, varid, _FillValue, NC_BYTE, 1, &fill_value);
         if (status != NC_ELATEFILL)
             printf("Error at line %d: expecting NC_ELATEFILL but got %s\n",__LINE__,nc_strerror(status));
         if (nc_close(ncid)) ERR;

         /* Open the file and check. */
         if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
         if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
         if (nvars_in != 1 || varids_in[0] != 0) ERR;
         if (nc_inq_varname(ncid, 0, name_in)) ERR;
         if (strcmp(name_in, VAR_NAME)) ERR;
         if (nc_inq_att(ncid, varid, _FillValue, &xtype_in, &len_in)) ERR;
         if (xtype_in != NC_BYTE || len_in != 1) ERR;
         if (nc_get_att(ncid, varid, _FillValue, &fill_value_in)) ERR;
         if (fill_value_in != fill_value) ERR;
         if (nc_close(ncid)) ERR;
      }

      SUMMARIZE_ERR;
      printf("**** testing simple fill value with data read...");
      {
         size_t start[NUM_DIMS], count[NUM_DIMS];
         signed char data = 99, data_in;

         /* Create a netcdf-4 file with one unlimited dim and one
          * var. Add fill value. */
         if (nc_create(FILE_NAME, cmode, &ncid)) ERR;
         if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
         if (nc_def_var(ncid, VAR_NAME, NC_BYTE, NUM_DIMS, dimids, &varid)) ERR;
         if (nc_put_att_schar(ncid, varid, _FillValue, NC_BYTE, 1, &fill_value)) ERR;
         if (nc_enddef(ncid)) ERR;

         /* Write the second record. */
         start[0] = 1;
         count[0] = 1;
         if (nc_put_vara_schar(ncid, varid, start, count, &data)) ERR;

         /* Read the first record, it should be the fill value. */
         start[0] = 0;
         if (nc_get_vara_schar(ncid, varid, start, count, &data_in)) ERR;
         if (data_in != fill_value) ERR;

         /* Read the second record, it should be the value we just wrote
          * there. */
         start[0] = 1;
         if (nc_get_vara_schar(ncid, varid, start, count, &data_in)) ERR;
         if (data_in != data) ERR;

         /* Close up. */
         if (nc_close(ncid)) ERR;

         /* Open the file and check. */
         if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

         /* This will not work because file was opened read-only. */
          if (nc_rename_var(ncid, 0, "something_very_new") != NC_EPERM) ERR;

         /* Check metadata. */
         if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
         if (nvars_in != 1 || varids_in[0] != 0) ERR;
         if (nc_inq_varname(ncid, 0, name_in)) ERR;
         if (strcmp(name_in, VAR_NAME)) ERR;

         /* Check fill value att. */
         if (nc_inq_att(ncid, varid, _FillValue, &xtype_in, &len_in)) ERR;
         if (xtype_in != NC_BYTE || len_in != 1) ERR;
         if (nc_get_att(ncid, varid, _FillValue, &fill_value_in)) ERR;
         if (fill_value_in != fill_value) ERR;

         /* Read the first record, it should be the fill value. */
         start[0] = 0;
         if (nc_get_vara_schar(ncid, varid, start, count, &data_in)) ERR;
         if (data_in != fill_value) ERR;

         /* Read the second record, it should be the value we just wrote
          * there. */
         start[0] = 1;
         if (nc_get_vara_schar(ncid, varid, start, count, &data_in)) ERR;
         if (data_in != data) ERR;

         if (nc_close(ncid)) ERR;
      }

      SUMMARIZE_ERR;
      printf("**** testing fill value with one other attribute...");

      {
         int losses_value = 192, losses_value_in;

         /* Create a netcdf-4 file with one dim and one var. Add another
          * attribute, then fill value. */
         if (nc_create(FILE_NAME, cmode, &ncid)) ERR;
         if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
         if (nc_def_var(ncid, VAR_NAME, NC_BYTE, NUM_DIMS, dimids, &varid)) ERR;
         if (nc_put_att_int(ncid, varid, LOSSES_NAME, NC_INT, 1, &losses_value)) ERR;
         if (nc_put_att_schar(ncid, varid, _FillValue, NC_BYTE, 1, &fill_value)) ERR;
         if (nc_close(ncid)) ERR;

         /* Open the file and check. */
         if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
         if (nc_inq_att(ncid, 0, LOSSES_NAME, &xtype_in, &len_in)) ERR;
         if (xtype_in != NC_INT || len_in != 1) ERR;
         if (nc_get_att(ncid, 0, LOSSES_NAME, &losses_value_in)) ERR;
         if (losses_value_in != losses_value) ERR;
         if (nc_inq_att(ncid, 0, _FillValue, &xtype_in, &len_in)) ERR;
         if (xtype_in != NC_BYTE || len_in != 1) ERR;
         if (nc_get_att(ncid, 0, _FillValue, &fill_value_in)) ERR;
         if (fill_value_in != fill_value) ERR;
         if (nc_inq_attid(ncid, 0, LOSSES_NAME, &attnum_in)) ERR;
         if (attnum_in != 0) ERR;
         if (nc_inq_attid(ncid, 0, _FillValue, &attnum_in)) ERR;
         if (attnum_in != 1) ERR;
         if (nc_close(ncid)) ERR;
      }

      SUMMARIZE_ERR;
      printf("**** testing fill value with three other attributes...");
      {
#define NUM_LEADERS 3
         char leader[NUM_LEADERS][NC_MAX_NAME + 1] = {"hair_length_of_strategoi",
                                                      "hair_length_of_Miltiades",
                                                      "hair_length_of_Darius_I"};
         short hair_length[NUM_LEADERS] = {3, 11, 4};
         short short_in;
         int a;

         /* Create a netcdf file with one dim and one var. Add 3
          * attributes, then fill value. */
         if (nc_create(FILE_NAME, cmode, &ncid)) ERR;
         if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
         if (nc_def_var(ncid, VAR_NAME, NC_BYTE, NUM_DIMS, dimids, &varid)) ERR;
         for (a = 0; a < NUM_LEADERS; a++)
            if (nc_put_att_short(ncid, varid, leader[a], NC_SHORT, 1, &hair_length[a])) ERR;
         if (nc_put_att_schar(ncid, varid, _FillValue, NC_BYTE, 1, &fill_value)) ERR;
         if (nc_close(ncid)) ERR;

         /* Open the file. */
         if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

         /* Check our three hair-related attributes. */
         for (a = 0; a < NUM_LEADERS; a++)
         {
            if (nc_inq_att(ncid, 0, leader[a], &xtype_in, &len_in)) ERR;
            if (xtype_in != NC_SHORT || len_in != 1) ERR;
            if (nc_get_att(ncid, 0, leader[a], &short_in)) ERR;
            if (short_in != hair_length[a]) ERR;
            if (nc_inq_attid(ncid, 0, leader[a], &attnum_in)) ERR;
            if (attnum_in != a) ERR;
         }

         /* Check our fill value attribute. */
         if (nc_inq_att(ncid, 0, _FillValue, &xtype_in, &len_in)) ERR;
         if (xtype_in != NC_BYTE || len_in != 1) ERR;
         if (nc_get_att(ncid, 0, _FillValue, &fill_value_in)) ERR;
         if (fill_value_in != fill_value) ERR;

         if (nc_close(ncid)) ERR;
      }

      SUMMARIZE_ERR;
      printf("**** testing fill value with simple example...");
      {
/* Dims stuff. */
#define NDIMS 3
#define VAR_DIMS 3
#define DIM_A "dim1"
#define DIM_A_LEN 4
#define DIM_B "dim2"
#define DIM_B_LEN 3
#define DIM_C "dim3"
#define DIM_C_LEN NC_UNLIMITED

/* Var stuff. */
#define CXX_VAR_NAME "P"

/* Att stuff. */
#define NUM_ATTS 4
#define LONG_NAME "long_name"
#define PRES_MAX_WIND "pressure at maximum wind"
#define UNITS "units"
#define HECTOPASCALS "hectopascals"

         int dimid[NDIMS], var_dimids[VAR_DIMS] = {2, 1, 0};
         float fill_value = -9999.0f;
         char long_name[] = PRES_MAX_WIND;

         if (nc_create(FILE_NAME, cmode, &ncid)) ERR;

         /* Create dims. */
         if (nc_def_dim(ncid, DIM_A, DIM_A_LEN, &dimid[0])) ERR;
         if (nc_def_dim (ncid, DIM_B, DIM_B_LEN, &dimid[1])) ERR;
         if (nc_def_dim(ncid, DIM_C, DIM_C_LEN, &dimid[2])) ERR;

         /* Create var. */
         if (nc_def_var(ncid, CXX_VAR_NAME, NC_FLOAT, VAR_DIMS,
                        var_dimids, &varid)) ERR;
         if (varid) ERR;

         if (nc_put_att(ncid, varid, LONG_NAME, NC_CHAR, strlen(long_name) + 1,
                        long_name)) ERR;
         if (nc_put_att(ncid, varid, UNITS, NC_CHAR, strlen(UNITS) + 1,
                        UNITS)) ERR;

         /* Check to ensure the atts have their expected attnums. */
         if (nc_inq_attid(ncid, 0, LONG_NAME, &attnum_in)) ERR;
         if (attnum_in != 0) ERR;
         if (nc_inq_attid(ncid, 0, UNITS, &attnum_in)) ERR;
         if (attnum_in != 1) ERR;

         /* Now add a fill value. This will acutually cause HDF5 to
          * destroy the dataset and recreate it, recreating also the
          * three attributes that are attached to it. */
         if (nc_put_att(ncid, varid, _FillValue, NC_FLOAT,
                        1, &fill_value)) ERR;

         /* Check to ensure the atts have their expected attnums. */
         if (nc_inq_attid(ncid, 0, LONG_NAME, &attnum_in)) ERR;
         if (attnum_in != 0) ERR;
         if (nc_inq_attid(ncid, 0, UNITS, &attnum_in)) ERR;
         if (attnum_in != 1) ERR;

         if (nc_close(ncid)) ERR;

         /* Open the file and check. */
         if (nc_open(FILE_NAME, 0, &ncid)) ERR;
         if (nc_inq_attid(ncid, 0, LONG_NAME, &attnum_in)) ERR;
         if (attnum_in != 0) ERR;
         if (nc_inq_attid(ncid, 0, UNITS, &attnum_in)) ERR;
         if (attnum_in != 1) ERR;
         if (nc_inq_attid(ncid, 0, _FillValue, &attnum_in)) ERR;
         if (attnum_in != 2) ERR;

         if (nc_close(ncid)) ERR;
      }
      SUMMARIZE_ERR;

#ifndef NO_NETCDF_2
      /* The following test is an attempt to recreate a problem occurring
         in the cxx tests. The file is created in c++ in nctsts.cpp. */
      printf("**** testing fill value with example from cxx tests in v2 api...");
      {
/* Dims stuff. */
#define NDIMS_1 4
#define VAR_DIMS 3
#define LAT "lat"
#define LAT_LEN 4
#define LON "lon"
#define LON_LEN 3
#define FRTIMED "frtimed"
#define FRTIMED_LEN NC_UNLIMITED
#define TIMELEN "timelen"
#define TIMELEN_LEN 20

/* Var stuff. */
#define CXX_VAR_NAME "P"

/* Att stuff. */
#define NUM_ATTS 4
#define LONG_NAME "long_name"
#define UNITS "units"

         int dimid[NDIMS_1], var_dimids[VAR_DIMS] = {2, 0, 1};
         float fill_value = -9999.0f;
         char long_name[] = PRES_MAX_WIND;
         int i;

         ncid = nccreate(FILE_NAME, NC_NETCDF4);

         /* Create dims. */
         dimid[0] = ncdimdef(ncid, LAT, LAT_LEN);
         dimid[1] = ncdimdef(ncid, LON, LON_LEN);
         dimid[2] = ncdimdef(ncid, FRTIMED, FRTIMED_LEN);
         dimid[3] = ncdimdef(ncid, TIMELEN, TIMELEN_LEN);

         /* Just check our dimids to see that they are correct. */
         for (i = 0; i < NDIMS_1; i++)
            if (dimid[i] != i) ERR;

         /* Create var. */
         varid = ncvardef(ncid, CXX_VAR_NAME, NC_FLOAT, VAR_DIMS, var_dimids);
         if (varid) ERR;

         /* Add three atts to the var, long_name, units, and
          * valid_range. */
         if (nc_put_att(ncid, varid, LONG_NAME, NC_CHAR, strlen(long_name) + 1,
                        long_name)) ERR;
         if (nc_put_att(ncid, varid, UNITS, NC_CHAR, strlen(UNITS) + 1,
                        UNITS)) ERR;

         /* Check to ensure the atts have their expected attnums. */
         if (nc_inq_attid(ncid, 0, LONG_NAME, &attnum_in)) ERR;
         if (attnum_in != 0) ERR;
         if (nc_inq_attid(ncid, 0, UNITS, &attnum_in)) ERR;
         if (attnum_in != 1) ERR;

         /* Now add a fill value. This will acutually cause HDF5 to
          * destroy the dataset and recreate it, recreating also the
          * three attributes that are attached to it. */
	 ncattput(ncid, varid, _FillValue, NC_FLOAT, 1, &fill_value);

         /* Check to ensure the atts have their expected attnums. */
         if (nc_inq_attid(ncid, 0, LONG_NAME, &attnum_in)) ERR;
         if (attnum_in != 0) ERR;
         if (nc_inq_attid(ncid, 0, UNITS, &attnum_in)) ERR;
         if (attnum_in != 1) ERR;
         if (nc_inq_attid(ncid, 0, _FillValue, &attnum_in)) ERR;
         if (attnum_in != 2) ERR;

         ncclose(ncid);

         /* Open the file and check. */
         ncid = ncopen(FILE_NAME, 0);
         if (nc_inq_attid(ncid, 0, LONG_NAME, &attnum_in)) ERR;
         if (attnum_in != 0) ERR;
         if (nc_inq_attid(ncid, 0, UNITS, &attnum_in)) ERR;
         if (attnum_in != 1) ERR;
         if (nc_inq_attid(ncid, 0, _FillValue, &attnum_in)) ERR;
         if (attnum_in != 2) ERR;
         ncclose(ncid);
      }
      SUMMARIZE_ERR;
#endif /* NO_NETCDF_2 */
   }

   printf("**** testing create order varids...");

#define UNITS "units"
#define DIMNAME "x"
#define VARNAME "data"
   {
      /* This test contributed by Jeff Whitaker of NOAA - Thanks Jeff! */
      int ncid, dimid, varid, xvarid;
      char units[] = "zlotys";

      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIMNAME, 1, &dimid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_redef(ncid)) ERR;

      /* Check that these netCDF-4 things will fail on this classic
       * model file. */
      if (nc_def_var(ncid, DIMNAME, NC_UINT, 1, &dimid, &xvarid) != NC_ESTRICTNC3) ERR;
      if (nc_def_var(ncid, DIMNAME, NC_INT, NC_MAX_VAR_DIMS + 1, &dimid,
                     &xvarid) != NC_EMAXDIMS) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, DIMNAME, NC_INT, 1, &dimid, &xvarid) != NC_ENOTINDEFINE) ERR;
      if (nc_redef(ncid)) ERR;

      /* Define the variable for the test. */
      if (nc_def_var(ncid, DIMNAME, NC_INT, 1, &dimid, &xvarid)) ERR;
      if (nc_put_att_text(ncid, xvarid, UNITS, strlen(units), units)) ERR;
      if (nc_def_var(ncid, VARNAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
#define RANK_wind 1
   printf("**** testing simple variable renaming...");
   {
      /* This test contributed by Jeff Whitaker of NOAA - Thanks Jeff! */
      int  ncid, lat_dim, time_dim, lon_dim, wind_id;
      size_t lat_len = 73, time_len = 10, lon_len = 145;
      int cdf_goober[1];

/*      if (nc_set_default_format(NC_FORMAT_NETCDF4, NULL)) ERR;*/
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* define dimensions */
      if (nc_def_dim(ncid, "a", lon_len, &lon_dim)) ERR;
      if (nc_def_dim(ncid, "b", lat_len, &lat_dim)) ERR;
      if (nc_def_dim(ncid, "c", time_len, &time_dim)) ERR;

      if (nc_put_att_text(ncid, NC_GLOBAL, "a", 3, "bar")) ERR;
      cdf_goober[0] = 2;
      if (nc_put_att_int(ncid, NC_GLOBAL, "b", NC_INT, 1, cdf_goober)) ERR;

      /* define variables */
      if (nc_def_var(ncid, "aa", NC_FLOAT, RANK_wind, &lon_dim, &wind_id)) ERR;
      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

      /* These won't work. */
      if (nc_rename_var(ncid + TEST_VAL_42, 0, "az") != NC_EBADID) ERR;
      if (nc_rename_var(ncid + MILLION, 0, "az") != NC_EBADID) ERR;

      /* Rename the var. */
      if (nc_rename_var(ncid, 0, "az")) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("**** testing dimension and variable renaming...");
   {
      /* This test based on code contributed by Jeff Whitaker of NOAA
       * - Thanks Jeff! */
      int  ncid, lat_dim, time_dim, lon_dim, wind_id, temp2_id;
      size_t lat_len = 73, time_len = 10, lon_len = 145;
      int wind_dims[RANK_wind], wind_slobber[1], cdf_goober[1];
      char too_long_name[NC_MAX_NAME + 2];

      /* Set up a name that is too long for netCDF. */
      memset(too_long_name, 'a', NC_MAX_NAME + 1);
      too_long_name[NC_MAX_NAME + 1] = 0;

      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;

      /* define dimensions */
      if (nc_def_dim(ncid, "lon", lon_len, &lon_dim)) ERR;
      if (nc_def_dim(ncid, "lat", lat_len, &lat_dim)) ERR;
      if (nc_def_dim(ncid, "time", time_len, &time_dim)) ERR;

      if (nc_put_att_text(ncid, NC_GLOBAL, "foo", 3, "bar")) ERR;
      cdf_goober[0] = 2;
      if (nc_put_att_int(ncid, NC_GLOBAL, "goober", NC_INT, 1, cdf_goober)) ERR;

      /* define variables */
      wind_dims[0] = lon_dim;
      if (nc_def_var(ncid, "temp", NC_FLOAT, RANK_wind, wind_dims, &wind_id)) ERR;
      if (nc_def_var(ncid, "temp2", NC_FLOAT, RANK_wind, wind_dims, &temp2_id)) ERR;

      if (nc_put_att_text(ncid, wind_id, "bar", 3, "foo")) ERR;
      wind_slobber[0] = 3;
      if (nc_put_att_int(ncid, wind_id, "slobber", NC_INT, 1, wind_slobber)) ERR;
      if (nc_close(ncid)) ERR;

      /* re-open dataset*/
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_dimid(ncid, "lon", &lon_dim)) ERR;

      /* rename dimension */
      if (nc_rename_dim(ncid, lon_dim, "longitude")) ERR;
      if (nc_inq_varid(ncid, "temp", &wind_id)) ERR;

      /* These won't work due to bad paramters. */
      if (nc_rename_var(ncid + MILLION, wind_id, "wind") != NC_EBADID) ERR;
      if (nc_rename_var(ncid, wind_id + TEST_VAL_42, "wind") != NC_ENOTVAR) ERR;
      if (nc_rename_var(ncid, -TEST_VAL_42, "wind") != NC_ENOTVAR) ERR;
      if (nc_rename_var(ncid, wind_id, BAD_NAME) != NC_EBADNAME) ERR;
      if (nc_rename_var(ncid, wind_id, too_long_name) != NC_EMAXNAME) ERR;
      if (nc_rename_var(ncid, wind_id, "temp2") != NC_ENAMEINUSE) ERR;
      if (nc_rename_var(ncid, wind_id, "windy") != NC_ENOTINDEFINE) ERR;

      /* rename variable */
      if (nc_rename_var(ncid, wind_id, "wind")) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

#define VAR_DIMS2 2
   printf("*** testing 2D array of NC_FLOAT with v2 API...");
   {
      int dimid[VAR_DIMS2];
      int ndims, nvars, natts, recdim;

      ncid = nccreate(FILE_NAME, NC_NETCDF4);

      /* Create dims. */
      dimid[0] = ncdimdef(ncid, LAT, LAT_LEN);
      dimid[1] = ncdimdef(ncid, LON, LON_LEN);

      /* Create var. */
      varid = ncvardef(ncid, CXX_VAR_NAME, NC_FLOAT, VAR_DIMS2, dimid);
      if (varid != 0) ERR;

      ncclose(ncid);

      /* Open the file and check. */
      ncid = ncopen(FILE_NAME, 0);
      ncinquire (ncid, &ndims, &nvars, &natts, &recdim);
      if (nvars != 1 || ndims != 2 || natts != 0 || recdim != -1) ERR;
      ncclose(ncid);
   }
   SUMMARIZE_ERR;

#define NDIMS 3
#define NNAMES 4
#define NLINES 13
   printf("**** testing funny names for netCDF-4...");
   {
      int  ncid, wind_id;
      size_t len[NDIMS] = {7, 3, 1};
      int dimids[NDIMS], dimids_in[NDIMS], ndims_in;
      char funny_name[NNAMES][NC_MAX_NAME] = {"\a\t", "\f\n", "\r\v", "\b"};
      char serious_name[NNAMES][NC_MAX_NAME] = {"name1", "name2", "name3", "name4"};
      char name_in[NC_MAX_NAME + 1];
      char *speech[NLINES] = {"who would fardels bear, ",
			      "To grunt and sweat under a weary life, ",
			      "But that the dread of something after death, ",
			      "The undiscover'd country from whose bourn ",
			      "No traveller returns, puzzles the will ",
			      "And makes us rather bear those ills we have ",
			      "Than fly to others that we know not of? ",
			      "Thus conscience does make cowards of us all; ",
			      "And thus the native hue of resolution ",
			      "Is sicklied o'er with the pale cast of thought, ",
			      "And enterprises of great pith and moment ",
			      "With this regard their currents turn awry, ",
			      "And lose the name of action."};
      char *speech_in[NLINES];
      int i;
      unsigned short nlines = NLINES;
      unsigned int nlines_in;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions. Funny names are rejected, serious names work. */
      for (i = 0; i < NDIMS; i++)
      	 if (nc_def_dim(ncid, funny_name[i], len[i], &dimids[i]) != NC_EBADNAME) ERR;
      for (i = 0; i < NDIMS; i++)
      	 if (nc_def_dim(ncid, serious_name[i], len[i], &dimids[i])) ERR;

      /* Write some global atts. Funny names are rejected, serious names work. */
      if (nc_put_att_string(ncid, NC_GLOBAL, funny_name[0], NLINES,
			    (const char **)speech) != NC_EBADNAME) ERR;
      if (nc_put_att_ushort(ncid, NC_GLOBAL, funny_name[1], NC_UINT, 1, &nlines) != NC_EBADNAME) ERR;
      if (nc_put_att_string(ncid, NC_GLOBAL, serious_name[0], NLINES,
			    (const char **)speech)) ERR;
      if (nc_put_att_ushort(ncid, NC_GLOBAL, serious_name[1], NC_UINT, 1, &nlines)) ERR;

      /* Define variables. Funny name fails, seriousness wins the day! */
      if (nc_def_var(ncid, funny_name[3], NC_INT64, NDIMS, dimids, &wind_id) != NC_EBADNAME) ERR;
      if (nc_def_var(ncid, serious_name[3], NC_INT64, NDIMS, dimids, &wind_id)) ERR;

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != NDIMS) ERR;
      for (i = 0; i < NDIMS; i++)
      {
      	 if (dimids_in[i] != i) ERR;
      	 if (nc_inq_dimname(ncid, i, name_in)) ERR;
      	 if (strcmp(name_in, serious_name[i])) ERR;
      }

      if (nc_get_att_string(ncid, NC_GLOBAL, serious_name[0], (char **)speech_in)) ERR;
      for (i = 0; i < NLINES; i++)
      	 if (strcmp(speech_in[i], speech[i])) ERR;
      if (nc_get_att_uint(ncid, NC_GLOBAL, serious_name[1], &nlines_in)) ERR;
      if (nlines_in != NLINES) ERR;
      if (nc_free_string(NLINES, (char **)speech_in)) ERR;
      if (nc_inq_varname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, serious_name[3])) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing endianness...");

#define NDIMS4 1
#define DIM4_NAME "Joe"
#define VAR_NAME4 "Ed"
#define DIM4_LEN 10
   {
      int dimids[NDIMS4], dimids_in[NDIMS4];
      int varid, varid1;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data[DIM4_LEN], data_in[DIM4_LEN];
      int endian_in;
      int i;

      for (i = 0; i < DIM4_LEN; i++)
         data[i] = i;

      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM4_NAME, DIM4_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;
      if (nc_def_var(ncid, VAR_NAME4, NC_INT, NDIMS4, dimids, &varid)) ERR;
      if (nc_def_var_endian(ncid, varid, NC_ENDIAN_BIG)) ERR;
      if (varid != 0) ERR;
      if (nc_put_var_int(ncid, varid, data)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS4 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;

      /* Test some bad parameter values. */
      if (nc_inq_var(ncid + MILLION, 0, name_in, &xtype_in, &ndims,
                     dimids_in, &natts) != NC_EBADID) ERR;
      if (nc_inq_var(ncid + TEST_VAL_42, 0, name_in, &xtype_in, &ndims,
                     dimids_in, &natts) != NC_EBADID) ERR;
      if (nc_inq_var(ncid, -TEST_VAL_42, name_in, &xtype_in, &ndims,
                     dimids_in, &natts) != NC_ENOTVAR) ERR;
      if (nc_inq_var(ncid, 1, name_in, &xtype_in, &ndims,
                     dimids_in, &natts) != NC_ENOTVAR) ERR;
      if (nc_inq_var(ncid, TEST_VAL_42, name_in, &xtype_in, &ndims,
                     dimids_in, &natts) != NC_ENOTVAR) ERR;

      /* Now pass correct parameters. */
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims,
                     dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME4) || xtype_in != NC_INT ||
          ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
      if (nc_inq_var_endian(ncid, 0, &endian_in)) ERR;
      if (endian_in != NC_ENDIAN_BIG) ERR;

      /* This also works, uselessly. */
      if (nc_inq_var(ncid, 0, name_in, NULL, NULL, NULL, NULL)) ERR;
      
      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* This won't work. */
      if (nc_def_var(ncid, "this_wont_work", NC_BYTE, NDIMS4, dimids,
                     &varid1) != NC_EPERM) ERR;

      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS4 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims,
                     dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME4) || xtype_in != NC_INT ||
          ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
      if (nc_inq_var_endian(ncid, 0, &endian_in)) ERR;
      if (endian_in != NC_ENDIAN_BIG) ERR;
      if (nc_get_var_int(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM4_LEN; i++)
	 if (data[i] != data_in[i]) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("**** testing chunking and the chunk cache...");
   {
#define NDIMS5 1
#define DIM5_NAME "D5"
#define VAR_NAME5 "V5"
#define DIM5_LEN 1000
#define CACHE_SIZE 32000000
#define CACHE_NELEMS 1009
#define CACHE_PREEMPTION .75

      int dimids[NDIMS5], dimids_in[NDIMS5];
      int varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data[DIM5_LEN], data_in[DIM5_LEN];
      size_t chunksize[NDIMS5] = {5};
      size_t bad_chunksize[NDIMS5] = {-5};
      size_t chunksize_in[NDIMS5];
      int storage_in;
      size_t cache_size_in, cache_nelems_in;
      float cache_preemption_in;
      int i, d;

      for (i = 0; i < DIM5_LEN; i++)
         data[i] = i;

      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;

      /* Define the variable. */
      if (nc_def_var(ncid, VAR_NAME5, NC_INT, NDIMS5, dimids, &varid)) ERR;

      /* This will fail due to bad chunk size. */
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, bad_chunksize) !=
          NC_EBADCHUNK) ERR;

      /* Define the chunking. */
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksize)) ERR;

      /* Try to set var cache with bad parameters. They will be
       * rejected. */
      if (nc_set_var_chunk_cache(ncid + MILLION, varid, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION) != NC_EBADID) ERR;
      if (nc_set_var_chunk_cache(ncid + 1, varid, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION) != NC_EBADID) ERR;
      if (nc_set_var_chunk_cache(ncid, varid + TEST_VAL_42, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION) != NC_ENOTVAR) ERR;
      if (nc_set_var_chunk_cache(ncid, -TEST_VAL_42, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION) != NC_ENOTVAR) ERR;
      if (nc_set_var_chunk_cache(ncid, varid + 1, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION) != NC_ENOTVAR) ERR;
      if (nc_set_var_chunk_cache(ncid, varid, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION + TEST_VAL_42) != NC_EINVAL) ERR;
      if (nc_set_var_chunk_cache(ncid, varid, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION - TEST_VAL_42) != NC_EINVAL) ERR;

      /* Set the cache. */
      if (nc_set_var_chunk_cache(ncid, varid, CACHE_SIZE, CACHE_NELEMS, CACHE_PREEMPTION)) ERR;
      if (nc_put_var_int(ncid, varid, data)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS5 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME5) || xtype_in != NC_INT || ndims != 1 || natts != 0 ||
	  dimids_in[0] != 0) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      for (d = 0; d < NDIMS5; d++)
	 if (chunksize[d] != chunksize_in[d]) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_get_var_int(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM5_LEN; i++)
         if (data[i] != data_in[i])
	    ERR_RET;

      /* Check that some bad parameter values are rejected properly. */
      if (nc_get_var_chunk_cache(ncid + MILLION, varid, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in) != NC_EBADID) ERR;
      if (nc_get_var_chunk_cache(ncid, varid + TEST_VAL_42, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in) != NC_ENOTVAR) ERR;
      if (nc_get_var_chunk_cache(ncid, varid + 1, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in) != NC_ENOTVAR) ERR;
      if (nc_get_var_chunk_cache(ncid, -TEST_VAL_42, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in) != NC_ENOTVAR) ERR;
      if (nc_get_var_chunk_cache(ncid + 1, -TEST_VAL_42, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in) != NC_EBADID) ERR;

      /* Get the var chunk cache settings. */
      if (nc_get_var_chunk_cache(ncid, varid, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in)) ERR;
      if (cache_size_in != CACHE_SIZE || cache_nelems_in != CACHE_NELEMS ||
	  cache_preemption_in != CACHE_PREEMPTION) ERR;
      /* THis should also work, pointlessly. */
      if (nc_get_var_chunk_cache(ncid, varid, NULL, NULL, NULL)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS5 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME5) || xtype_in != NC_INT || ndims != 1 || natts != 0 ||
	  dimids_in[0] != 0) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      for (d = 0; d < NDIMS5; d++)
	 if (chunksize[d] != chunksize_in[d]) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_get_var_int(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM5_LEN; i++)
         if (data[i] != data_in[i])
	    ERR_RET;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("**** testing netCDF-4 functions on netCDF-3 files...");
   {
      int dimids[NDIMS5], dimids_in[NDIMS5];
      int varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data[DIM5_LEN], data_in[DIM5_LEN];
      size_t chunksize[NDIMS5] = {5};
      size_t chunksize_in[NDIMS5];
      int storage_in;
      size_t cache_size_in, cache_nelems_in;
      float cache_preemption_in;
      int i;

      for (i = 0; i < DIM5_LEN; i++)
         data[i] = i;

      /* Create a netcdf classic file with one dim and one var. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;
      if (nc_def_var(ncid, VAR_NAME5, NC_INT, NDIMS5, dimids, &varid)) ERR;

      /* These will return error. */
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksize) != NC_ENOTNC4) ERR;
      if (nc_set_var_chunk_cache(ncid, varid, CACHE_SIZE, CACHE_NELEMS,
                                 CACHE_PREEMPTION) != NC_ENOTNC4) ERR;

      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_int(ncid, varid, data)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS5 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME5) || xtype_in != NC_INT || ndims != 1 || natts != 0 ||
	  dimids_in[0] != 0) ERR;

      /* This call fails. */
      if (nc_get_var_chunk_cache(ncid, varid, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in) != NC_ENOTNC4) ERR;

      /* This call passes but does nothing. */
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;

      if (nc_get_var_int(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM5_LEN; i++)
         if (data[i] != data_in[i])
	    ERR_RET;

      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS5 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME5) || xtype_in != NC_INT || ndims != 1 || natts != 0 ||
	  dimids_in[0] != 0) ERR;

      /* This call fails. */
      if (nc_get_var_chunk_cache(ncid, varid, &cache_size_in, &cache_nelems_in,
				 &cache_preemption_in) != NC_ENOTNC4) ERR;

      /* This call passes but does nothing. */
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      
      if (nc_get_var_int(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM5_LEN; i++)
         if (data[i] != data_in[i])
	    ERR_RET;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("**** testing contiguous storage...");
   {
#define NDIMS6 1
#define DIM6_NAME "D5"
#define VAR_NAME6 "V5"
#define DIM6_LEN 100

      int dimids[NDIMS6], dimids_in[NDIMS6];
      int varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data[DIM6_LEN], data_in[DIM6_LEN];
      size_t chunksize_in[NDIMS6];
      int storage_in;
      int i;

      for (i = 0; i < DIM6_LEN; i++)
         data[i] = i;

      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM6_NAME, DIM6_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;
      if (nc_def_var(ncid, VAR_NAME6, NC_INT, NDIMS6, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CONTIGUOUS, NULL)) ERR;
      if (nc_put_var_int(ncid, varid, data)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS6 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME6) || xtype_in != NC_INT || ndims != 1 || natts != 0 ||
	  dimids_in[0] != 0) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      if (storage_in != NC_CONTIGUOUS) ERR;
      if (nc_get_var_int(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM6_LEN; i++)
         if (data_in[i] != data[i])
	    ERR_RET;
      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS6 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME6) || xtype_in != NC_INT || ndims != 1 || natts != 0 ||
	  dimids_in[0] != 0) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      if (storage_in != NC_CONTIGUOUS) ERR;
      if (nc_get_var_int(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM6_LEN; i++)
         if (data[i] != data_in[i])
	    ERR_RET;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("**** testing extreme numbers dude...");
   {
#define VAR_NAME7 "V5"
#define DIM6_LEN 100

      int varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
/*      unsigned long long data = 9223372036854775807ull, data_in;*/
      unsigned long long data = 9223372036854775817ull, data_in;

      /* Create a netcdf-4 file with scalar var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_var(ncid, VAR_NAME7, NC_UINT64, 0, NULL, &varid)) ERR;
      if (nc_put_var_ulonglong(ncid, varid, &data)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1 || varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, NULL, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME7) || xtype_in != NC_UINT64 || ndims != 0 || natts != 0) ERR;
      if (nc_get_var_ulonglong(ncid, varid, &data_in)) ERR;
      if (data_in != data) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1 || varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, NULL, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME7) || xtype_in != NC_UINT64 || ndims != 0 || natts != 0) ERR;
      if (nc_get_var_ulonglong(ncid, varid, &data_in)) ERR;
      if (data_in != data) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("**** testing error codes for name clashes...");
   {
#define GENERIC_NAME "bob"
      int ncid, varid, numgrps, ntypes;

      /* Create a netcdf-4 file with one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_var(ncid, GENERIC_NAME, NC_BYTE, 0, NULL, &varid)) ERR;

      /* These don'e work, because the name is already in use. Make
       * sure the correct error is returned. */
      if (nc_def_grp(ncid, GENERIC_NAME, NULL) != NC_ENAMEINUSE) ERR;
      if (nc_def_opaque(ncid, 1, GENERIC_NAME, NULL) != NC_ENAMEINUSE) ERR;

      /* Close it. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 1 || varids_in[0] != 0) ERR;
      if (nc_inq_varname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, GENERIC_NAME)) ERR;
      if (nc_inq_grps(ncid, &numgrps, NULL)) ERR;
      if (numgrps) ERR;
      if (nc_inq_typeids(ncid, &ntypes, NULL)) ERR;
      if (ntypes) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("**** testing error codes for name clashes some more...");

   {
#define GENERIC_NAME "bob"
      int ncid, varid, numgrps, ntypes;

      /* Create a netcdf-4 file with one type. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_opaque(ncid, 1, GENERIC_NAME, NULL)) ERR;

      /* These don'e work, because the name is already in use. Make
       * sure the correct error is returned. */
      if (nc_def_grp(ncid, GENERIC_NAME, NULL) != NC_ENAMEINUSE) ERR;
      if (nc_def_var(ncid, GENERIC_NAME, NC_BYTE, 0, NULL, &varid) != NC_ENAMEINUSE) ERR;

      /* Close it. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in) ERR;
      if (nc_inq_grps(ncid, &numgrps, NULL)) ERR;
      if (numgrps) ERR;
      if (nc_inq_typeids(ncid, &ntypes, NULL)) ERR;
      if (ntypes != 1) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing error codes for name clashes even more...");

   {
#define GENERIC_NAME "bob"
      int ncid, varid, numgrps, ntypes;

      /* Create a netcdf-4 file with one group. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, GENERIC_NAME, NULL)) ERR;

      /* These don'e work, because the name is already in use. Make
       * sure the correct error is returned. */
      if (nc_def_opaque(ncid, 1, GENERIC_NAME, NULL) != NC_ENAMEINUSE) ERR;
      if (nc_def_var(ncid, GENERIC_NAME, NC_BYTE, 0, NULL, &varid) != NC_ENAMEINUSE) ERR;

      /* Close it. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in) ERR;
      if (nc_inq_grps(ncid, &numgrps, NULL)) ERR;
      if (numgrps != 1) ERR;
      if (nc_inq_typeids(ncid, &ntypes, NULL)) ERR;
      if (ntypes) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing error code for too-large chunks...");
   {
#define NDIMS17 2
#define DIM17_NAME "personality"
#define DIM17_NAME_2 "good_looks"
#define VAR_NAME17 "ed"
#define DIM17_LEN 2147483644 /* max dimension size - 2GB - 4. */
#define DIM17_2_LEN 1000

      int dimids[NDIMS17], dimids_in[NDIMS17];
      int varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      size_t chunksize[NDIMS17] = {5, 5};
      size_t bad_chunksize[NDIMS17] = {5, DIM17_LEN};
      size_t chunksize_in[NDIMS17];
      int storage_in;
      int d;

      /* Create a netcdf-4 file with two dims and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM17_NAME, DIM17_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM17_NAME_2, DIM17_2_LEN, &dimids[1])) ERR;
      if (dimids[0] != 0 || dimids[1] != 1) ERR;
      if (nc_def_var(ncid, VAR_NAME17, NC_UINT64, NDIMS17, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, bad_chunksize) != NC_EBADCHUNK) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksize)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS17 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME17) || xtype_in != NC_UINT64 || ndims != 2 || natts != 0 ||
	  dimids_in[0] != 0 || dimids_in[1] != 1) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      for (d = 0; d < NDIMS17; d++)
	 if (chunksize[d] != chunksize_in[d]) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS17 || nvars != 1 || natts != 0 ||
          unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != 1) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME17) || xtype_in != NC_UINT64 || ndims != 2 || natts != 0 ||
	  dimids_in[0] != 0 || dimids_in[1] != 1) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
      for (d = 0; d < NDIMS17; d++)
	 if (chunksize[d] != chunksize_in[d]) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
#define NDIMS6 1
#define DIM8_NAME "num_monkeys"
#define DIM9_NAME "num_coconuts"
#define DIM9_LEN 10
#define VAR_NAME8 "John_Clayton"
#define VAR_NAME9 "Lord_Greystoke"
   printf("**** testing that contiguous storage can't be turned on for vars with unlimited dims or filters...");
   {
      int ncid;
      int dimids[NDIMS6];
      int varid, varid2;
      size_t chunksize_in[NDIMS6];
      int storage_in;

      /* Create a netcdf-4 file with one dim and some vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM8_NAME, NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR_NAME8, NC_INT, NDIMS6, dimids, &varid)) ERR;
      if (nc_def_dim(ncid, DIM9_NAME, DIM9_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR_NAME9, NC_INT, NDIMS6, dimids, &varid2)) ERR;
      if (nc_def_var_deflate(ncid, varid2, 0, 1, 4)) ERR;

      /* This won't work because of the umlimited dimension. */
      if (nc_def_var_chunking(ncid, varid, NC_CONTIGUOUS, NULL) != NC_EINVAL) ERR;

      /* This won't work because of the deflate filter. */
      if (nc_def_var_chunking(ncid, varid2, NC_CONTIGUOUS, NULL) != NC_EINVAL) ERR;

      /* Storage must be chunked because of unlimited dimension and
       * the deflate filter. */
      if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksize_in)) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_inq_var_chunking(ncid, varid2, &storage_in, chunksize_in)) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
#define NDIMS6 1
#define DIM8_NAME "num_monkeys"
#define DIM9_NAME "num_coconuts"
#define DIM9_LEN 10
#define VAR_NAME8 "John_Clayton"
#define VAR_NAME9 "Lord_Greystoke"
   printf("**** testing that contiguous storage can't be turned on for vars with unlimited dims or filters...");
   {
      int ncid;
      int dimids[NDIMS6];
      int varid, varid2;
      size_t chunksize_in[NDIMS6];
      int storage_in;

      /* Create a netcdf-4 file with one dim and some vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM8_NAME, NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR_NAME8, NC_INT, NDIMS6, dimids, &varid)) ERR;
      if (nc_def_dim(ncid, DIM9_NAME, DIM9_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR_NAME9, NC_INT, NDIMS6, dimids, &varid2)) ERR;
      if (nc_def_var_deflate(ncid, varid2, 0, 1, 4)) ERR;

      /* This won't work because of the umlimited dimension. */
      if (nc_def_var_chunking(ncid, varid, NC_CONTIGUOUS, NULL) != NC_EINVAL) ERR;

      /* This won't work because of the deflate filter. */
      if (nc_def_var_chunking(ncid, varid2, NC_CONTIGUOUS, NULL) != NC_EINVAL) ERR;

      /* Storage must be chunked because of unlimited dimension and
       * the deflate filter. */
      if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksize_in)) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_inq_var_chunking(ncid, varid2, &storage_in, chunksize_in)) ERR;
      if (storage_in != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
