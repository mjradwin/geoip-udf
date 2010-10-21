/*
 * MySQL User Defined Functions for MaxMind GeoIP API
 *
 * Copyright (c) 2010, Anchor Intelligence. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Anchor Intelligence nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * To compile:
 *
 * gcc -g -fPIC -I/usr/include/mysql -lGeoIP -shared -o udf_geoip.so udf_geoip.c 
 *
 */

static const char __ident[] __attribute__((unused)) =
  "$Id: udf_geoip.c 19817 2009-12-16 18:58:31Z ryan $";

#include <mysql.h>
#include <string.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

#define GEOIP_DATA_PATH "/usr/share/GeoIP/"

/* copied from unireg.h in mysql source */
#define MAX_MBWIDTH             3               /* Max multibyte sequence */
#define MAX_FIELD_CHARLENGTH    255
/* Max column width +1 */
#define MAX_FIELD_WIDTH         (MAX_FIELD_CHARLENGTH*MAX_MBWIDTH+1)

my_bool geoip_country_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_country_name_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_region_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_city_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_dma_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_area_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_latitude_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_longitude_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_postal_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_org_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);
my_bool geoip_isp_init(UDF_INIT *initid, UDF_ARGS *args, char *msg);

void geoip_country_code_deinit(UDF_INIT *initid);
void geoip_country_name_deinit(UDF_INIT *initid);
void geoip_region_deinit(UDF_INIT *initid);
void geoip_city_deinit(UDF_INIT *initid);
void geoip_dma_code_deinit(UDF_INIT *initid);
void geoip_area_code_deinit(UDF_INIT *initid);
void geoip_latitude_deinit(UDF_INIT *initid);
void geoip_longitude_deinit(UDF_INIT *initid);
void geoip_postal_code_deinit(UDF_INIT *initid);
void geoip_org_deinit(UDF_INIT *initid);
void geoip_isp_deinit(UDF_INIT *initid);

char *geoip_country_code(UDF_INIT *initid, UDF_ARGS *args, char *result,
			 unsigned long *length, char *is_null, char *error);
char *geoip_country_name(UDF_INIT *initid, UDF_ARGS *args, char *result,
			 unsigned long *length, char *is_null, char *error);
char *geoip_region(UDF_INIT *initid, UDF_ARGS *args, char *result,
		   unsigned long *length, char *is_null, char *error);
char *geoip_city(UDF_INIT *initid, UDF_ARGS *args, char *result,
		 unsigned long *length, char *is_null, char *error);
long long geoip_dma_code(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
			 char *error);
long long geoip_area_code(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
			  char *error);
double geoip_latitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		      char *error);
double geoip_longitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		       char *error);
char *geoip_postal_code(UDF_INIT *initid, UDF_ARGS *args, char *result,
			unsigned long *length, char *is_null, char *error);
char *geoip_org(UDF_INIT *initid, UDF_ARGS *args, char *result,
		unsigned long *length, char *is_null, char *error);
char *geoip_isp(UDF_INIT *initid, UDF_ARGS *args, char *result,
		unsigned long *length, char *is_null, char *error);

int latin1_to_utf8(char *dest, const char *src);

/* Init */

static my_bool check_args(const char *funcname,
			  UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    initid->maybe_null=1;
    if (args->arg_count != 1 || (args->arg_type[0] != STRING_RESULT &&
				 args->arg_type[0] != INT_RESULT))
    {
	strcpy(msg, "Wrong arguments to ");
	strcat(msg, funcname);
	return 1;
    }
    return 0;
}

static my_bool check_args_city(const char *funcname,
			       UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    int rv;
    GeoIP *gi_city;

    if ((rv = check_args(funcname, initid, args, msg)))
	return rv;

    if ((gi_city = GeoIP_open(GEOIP_DATA_PATH "GeoIPCity.dat",
			      GEOIP_STANDARD)) == NULL)
    {
	strcpy(msg, "Couldn't open GeoIPCity.dat");
	return 1;
    }

    initid->ptr = (void *)gi_city;
    return 0;
}

typedef struct {
    GeoIP *gi_country;
    GeoIP *gi_city;
} citycountry_ptrs_t;

static my_bool check_args_country(const char *funcname,
				  UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    int rv;
    citycountry_ptrs_t *data;

    if ((rv = check_args(funcname, initid, args, msg)))
	return rv;

    if (!(data = (citycountry_ptrs_t *) malloc(sizeof(citycountry_ptrs_t))))
    {
	strcpy(msg, "Couldn't allocate memory");
	return 1;
    }

    if ((data->gi_country = GeoIP_open(GEOIP_DATA_PATH "GeoIP.dat",
				       GEOIP_STANDARD)) == NULL)
    {
	strcpy(msg, "Couldn't open GeoIP.dat");
	return 1;
    }

    if ((data->gi_city = GeoIP_open(GEOIP_DATA_PATH "GeoIPCity.dat",
				    GEOIP_STANDARD)) == NULL)
    {
	strcpy(msg, "Couldn't open GeoIPCity.dat");
	return 1;
    }

    initid->ptr = (void *)data;

    return 0;
}

my_bool geoip_country_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    initid->max_length=3;
    return check_args_country("geoip_country_code", initid, args, msg);
}

my_bool geoip_country_name_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    initid->max_length=100;
    return check_args_country("geoip_country_name", initid, args, msg);
}

my_bool geoip_region_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    initid->max_length=100;
    return check_args_city("geoip_region", initid, args, msg);
}

my_bool geoip_city_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    initid->max_length=100;
    return check_args_city("geoip_city", initid, args, msg);
}

my_bool geoip_dma_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    return check_args_city("geoip_dma_code", initid, args, msg);
}

my_bool geoip_area_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    return check_args_city("geoip_area_code", initid, args, msg);
}

my_bool geoip_latitude_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    return check_args_city("geoip_latitude", initid, args, msg);
}

my_bool geoip_longitude_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    return check_args_city("geoip_longitude", initid, args, msg);
}

my_bool geoip_postal_code_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    initid->max_length=100;
    return check_args_city("geoip_postal_code", initid, args, msg);
}

my_bool geoip_org_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    int rv;

    if ((rv = check_args("geoip_org", initid, args, msg)))
	return rv;

    initid->max_length=60;
    if ((initid->ptr = (void*)GeoIP_open(GEOIP_DATA_PATH "GeoIPOrg.dat",
					 GEOIP_STANDARD)) == NULL)
    {
	strcpy(msg, "Couldn't open GeoIPOrg.dat");
	return 1;
    }

    return 0;
}

my_bool geoip_isp_init(UDF_INIT *initid, UDF_ARGS *args, char *msg)
{
    int rv;

    if ((rv = check_args("geoip_isp", initid, args, msg)))
	return rv;

    initid->max_length=60;
    if ((initid->ptr = (void*)GeoIP_open(GEOIP_DATA_PATH "GeoIPISP.dat",
					 GEOIP_STANDARD)) == NULL)
    {
	strcpy(msg, "Couldn't open GeoIPISP.dat");
	return 1;
    }

    return 0;
}

void geoip_country_code_deinit(UDF_INIT *initid)
{
    if (initid->ptr)
    {
	citycountry_ptrs_t *data = (citycountry_ptrs_t *)initid->ptr;
	GeoIP_delete(data->gi_country);
	GeoIP_delete(data->gi_city);
	free(data);
    }
}

void geoip_country_name_deinit(UDF_INIT *initid)
{
    if (initid->ptr)
    {
	citycountry_ptrs_t *data = (citycountry_ptrs_t *)initid->ptr;
	GeoIP_delete(data->gi_country);
	GeoIP_delete(data->gi_city);
	free(data);
    }
}

void geoip_region_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_city_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_dma_code_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_area_code_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_latitude_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_longitude_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_postal_code_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_org_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}

void geoip_isp_deinit(UDF_INIT *initid)
{
    if (initid->ptr) GeoIP_delete((GeoIP *)initid->ptr);
}


/* Implementation */

#define COPY_IPSTR(buf,argstr,arglen,retval) \
	if (!(arglen) || (arglen) > 15) \
	{ \
	    *is_null = 1; \
	    return (retval); \
	} \
	memcpy((buf), (argstr), (arglen)); \
	(buf)[(arglen)] = '\0'

char *geoip_country_code(UDF_INIT *initid, UDF_ARGS *args, char *result,
			 unsigned long *length, char *is_null, char *error)
{
    const char *country_code = NULL;
    size_t len;
    citycountry_ptrs_t *data = (citycountry_ptrs_t *)initid->ptr;

    if (data == NULL || data->gi_country == NULL)
    {
	*error = 1;
	return NULL;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return NULL;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], NULL);
	country_code = GeoIP_country_code_by_addr(data->gi_country, ipstr);
	if (country_code == NULL)
	{
	    GeoIPRecord *gir;
	    if ((gir = GeoIP_record_by_addr(data->gi_city, ipstr)) != NULL)
	    {
		country_code = gir->country_code;
		GeoIPRecord_delete(gir);
	    }
	}
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	country_code = GeoIP_country_code_by_ipnum(data->gi_country, *((long long *) args->args[0]));
	if (country_code == NULL)
	{
	    GeoIPRecord *gir;
	    if ((gir = GeoIP_record_by_ipnum(data->gi_city, *((long long *) args->args[0]))) != NULL)
	    {
		country_code = gir->country_code;
		GeoIPRecord_delete(gir);
	    }
	}
    }

    if (country_code != NULL && ((len = strlen(country_code)) != 0))
    {
	*length = latin1_to_utf8(result, country_code);
	return result;
    }
    else
    {
	*is_null = 1;
	return result;
    }
}

char *geoip_country_name(UDF_INIT *initid, UDF_ARGS *args, char *result,
			 unsigned long *length, char *is_null, char *error)
{
    const char *country_name = NULL;
    size_t len;
    citycountry_ptrs_t *data = (citycountry_ptrs_t *)initid->ptr;

    if (data == NULL || data->gi_country == NULL)
    {
	*error = 1;
	return NULL;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return NULL;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], NULL);
	country_name = GeoIP_country_name_by_addr(data->gi_country, ipstr);
	if (country_name == NULL)
	{
	    GeoIPRecord *gir;
	    if ((gir = GeoIP_record_by_addr(data->gi_city, args->args[0])) != NULL)
	    {
		country_name = gir->country_name;
		GeoIPRecord_delete(gir);
	    }
	}
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	country_name = GeoIP_country_name_by_ipnum(data->gi_country, *((long long *) args->args[0]));
	if (country_name == NULL)
	{
	    GeoIPRecord *gir;
	    if ((gir = GeoIP_record_by_ipnum(data->gi_city, *((long long *) args->args[0]))) != NULL)
	    {
		country_name = gir->country_name;
		GeoIPRecord_delete(gir);
	    }
	}
    }

    if (country_name != NULL && ((len = strlen(country_name)) != 0))
    {
	*length = latin1_to_utf8(result, country_name);
	return result;
    }
    else
    {
	*is_null = 1;
	return result;
    }
}

char *geoip_region(UDF_INIT *initid, UDF_ARGS *args, char *result,
		   unsigned long *length, char *is_null, char *error)
{
    GeoIPRecord *gir;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return NULL;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return NULL;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], NULL);
	gir = GeoIP_record_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	gir = GeoIP_record_by_ipnum((GeoIP *)initid->ptr,
				    *((long long *) args->args[0]));
    }

    if (gir != NULL && gir->region != NULL)
    {
	*length = latin1_to_utf8(result, gir->region);
	GeoIPRecord_delete(gir);	
	return result;
    }
    else
    {
	*is_null = 1;
	return result;
    }
}

char *geoip_city(UDF_INIT *initid, UDF_ARGS *args, char *result,
		 unsigned long *length, char *is_null, char *error)
{
    GeoIPRecord *gir;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return NULL;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return NULL;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], NULL);
	gir = GeoIP_record_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	gir = GeoIP_record_by_ipnum((GeoIP *)initid->ptr,
				    *((long long *) args->args[0]));
    }

    if (gir != NULL && gir->city != NULL)
    {
	*length = latin1_to_utf8(result, gir->city);
	GeoIPRecord_delete(gir);
	return result;
    }
    else
    {
	*is_null = 1;
	return result;
    }
}

long long geoip_dma_code(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
			 char *error)
{
    GeoIPRecord *gir;
    long long dma_code;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return -1;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return -1;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], -1);
	gir = GeoIP_record_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	gir = GeoIP_record_by_ipnum((GeoIP *)initid->ptr,
				    *((long long *) args->args[0]));
    }

    if (gir != NULL)
    {
	dma_code = (long long) gir->dma_code;
	GeoIPRecord_delete(gir);
	return dma_code;
    }
    else
    {
	*is_null = 1;
	return -1;
    }
}

long long geoip_area_code(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
			  char *error)
{
    GeoIPRecord *gir;
    long long area_code;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return -1;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return -1;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], -1);
	gir = GeoIP_record_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	gir = GeoIP_record_by_ipnum((GeoIP *)initid->ptr,
				    *((long long *) args->args[0]));
    }

    if (gir != NULL)
    {
	area_code = (long long) gir->area_code;
	GeoIPRecord_delete(gir);
	return area_code;
    }
    else
    {
	*is_null = 1;
	return -1;
    }
}

double geoip_latitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		      char *error)
{
    GeoIPRecord *gir;
    double latitude;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return -1.0;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return -1.0;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], -1.0);
	gir = GeoIP_record_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	gir = GeoIP_record_by_ipnum((GeoIP *)initid->ptr,
				    *((long long *) args->args[0]));
    }

    if (gir != NULL)
    {
	latitude = (double) gir->latitude;
	GeoIPRecord_delete(gir);
	return latitude;
    }
    else
    {
	*is_null = 1;
	return -1.0;
    }
}

double geoip_longitude(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		       char *error)
{
    GeoIPRecord *gir;
    double longitude;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return -1.0;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return -1.0;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], -1.0);
	gir = GeoIP_record_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	gir = GeoIP_record_by_ipnum((GeoIP *)initid->ptr,
				    *((long long *) args->args[0]));
    }

    if (gir != NULL)
    {
	longitude = (double) gir->longitude;
	GeoIPRecord_delete(gir);
	return longitude;
    }
    else
    {
	*is_null = 1;
	return -1.0;
    }
}

char *geoip_postal_code(UDF_INIT *initid, UDF_ARGS *args, char *result,
			unsigned long *length, char *is_null, char *error)
{
    GeoIPRecord *gir;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return NULL;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return NULL;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], NULL);
	gir = GeoIP_record_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	gir = GeoIP_record_by_ipnum((GeoIP *)initid->ptr,
				    *((long long *) args->args[0]));
    }

    if (gir != NULL && gir->postal_code != NULL)
    {
	*length = latin1_to_utf8(result, gir->postal_code);
	GeoIPRecord_delete(gir);
	return result;
    }
    else
    {
	*is_null = 1;
	return result;
    }
}

char *geoip_org(UDF_INIT *initid, UDF_ARGS *args, char *result,
		unsigned long *length, char *is_null, char *error)
{
    char *org;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return NULL;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return NULL;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], NULL);
	org = GeoIP_name_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	org = GeoIP_name_by_ipnum((GeoIP *)initid->ptr,
				  *((long long *) args->args[0]));
    }

    if (org != NULL)
    {
	*length = latin1_to_utf8(result, org);
	free(org);
	return result;
    }
    else
    {
	*is_null = 1;
	return result;
    }
}

char *geoip_isp(UDF_INIT *initid, UDF_ARGS *args, char *result,
		unsigned long *length, char *is_null, char *error)
{
    char *isp;

    if (initid->ptr == NULL)
    {
	*error = 1;
	return NULL;
    }

    if (args->args[0] == NULL)
    {
	*is_null = 1;
	return NULL;
    }

    if (args->arg_type[0] == STRING_RESULT)
    {
	char ipstr[16];
	COPY_IPSTR(ipstr, args->args[0], args->lengths[0], NULL);
	isp = GeoIP_name_by_addr((GeoIP *)initid->ptr, ipstr);
    }
    else /* args->arg_type[0] == INT_RESULT */
    {
	isp = GeoIP_name_by_ipnum((GeoIP *)initid->ptr,
				  *((long long *) args->args[0]));
    }

    if (isp != NULL)
    {
	*length = latin1_to_utf8(result, isp);
	free(isp);
	return result;
    }
    else
    {
	*is_null = 1;
	return result;
    }
}

/*
 * Reads the the ISO-8859-1 (Latin 1) string "src" and
 * writes the UTF-8 version of the string to "dest".
 * NOTE: dest is *not* null terminated
 *
 * Returns the number of bytes written to "dest"
 */
int latin1_to_utf8(char *dest, const char *src) {
	const char* s = src;
	char *t = dest;
	while (*s != '\0') {
		unsigned char c = *s++;
		if (c & 0x80) {
			/* high bit is set, so expand to utf 8 characters */
			*t++ = 0xC0 | (c >> 6);
			*t++ = 0x80 | (0x3F & c);
		} else {
			*t++ = c;
		}
		/* avoid overwriting the end of the output array */
		if (MAX_FIELD_WIDTH == dest - t) {
			break;
		}
	}
	return t - dest;
}
