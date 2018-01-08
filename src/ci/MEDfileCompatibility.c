/*  This file is part of MED.
 *
 *  COPYRIGHT (C) 1999 - 2016  EDF R&D, CEA/DEN
 *  MED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with MED.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <med.h>
#include <med_config.h>
#include <med_outils.h>


/**\ingroup MEDfile
  \brief \MEDfileCompatibilityBrief
  \param filename \filename
  \param hdfok \hdfok
  \param medok \medok
  \retval med_err \error
  \details \MEDfileCompatibilityDetails
 */

med_err
MEDfileCompatibility(const char* const filename,
		     med_bool* const hdfok,
		     med_bool* const medok)
{
  med_err _ret = -1;
  med_idt _fid =  0;
  med_idt _id  =  0;
  med_int _major = 0;
  med_int _minor = 0;
  med_int _hmajeur=0, _hmineur=0, _hrelease=0;
  med_int _hversionMMR = 0;
  med_int _fversionMM  = 0;


  
  _MEDmodeErreurVerrouiller();

  *hdfok = MED_FALSE;
  *medok = MED_FALSE;

  /* Si le fichier n'est pas un fichier hdf retourne en indiquant hdfok=false et medok=false */
  if  ( H5Fis_hdf5(filename) < 0 )  {_ret=0;goto ERROR;};

  if (MEDlibraryHdfNumVersion(&_hmajeur, &_hmineur, &_hrelease) < 0) {
    MED_ERR_(_ret,MED_ERR_CALL,MED_ERR_API,"MEDlibraryNumVersion");
    goto ERROR;
  }

  /* TODO : Vérifier si la version mineur d'HDF du fichier est supérieur à la version d'HDF utilisée  */
  /** Je n'ai pas trouvé de mécanisme HDF pour trouver la version du fichier */
  
  /* Si la bibliothèque HDF est conforme à la version HDF de référence de cette version de la bibliothèque MED renvoie hdfok=false et medok=false */
  /* Il se peut tout de même que cette version d'HDF soit partiellement utilisable par la bibliothèque ... on test aussi la version du modèle med */
  _hversionMMR=10000*_hmajeur+100*_hmineur+_hrelease;
  /* ISCRUTE(_hversionMMR); */
  /* ISCRUTE(HDF_VERSION_NUM_REF); */
  if ( (_hversionMMR >= HDF_VERSION_NUM_REF) && (_hmineur == HDF_VERSION_MINOR_REF) ) *hdfok = MED_TRUE;
  
  /* compatibility with med */
  if ((_fid = _MEDfileOpen((char *)filename,MED_ACC_RDONLY)) < 0) {
    MED_ERR_(_ret,MED_ERR_OPEN,MED_ERR_FILE,filename);
    /* SSCRUTE(filename); */
    goto ERROR;
  }

  if ((_id = _MEDdatagroupOuvrir(_fid,MED_INFOS)) < 0) {
    MED_ERR_(_ret,MED_ERR_OPEN,MED_ERR_DATAGROUP,MED_INFOS);
    goto ERROR;
  }

  if (_MEDattrEntierLire(_id,MED_MAJOR_NAME,&_major) < 0) {
    MED_ERR_(_ret,MED_ERR_READ,MED_ERR_ATTRIBUTE,MED_MAJOR_NAME);
    goto ERROR;
  }

  if (_MEDattrEntierLire(_id,MED_MINOR_NAME,&_minor) < 0) {
    MED_ERR_(_ret,MED_ERR_READ,MED_ERR_ATTRIBUTE,MED_MINOR_NAME);
    goto ERROR;
  }

  _fversionMM = 100*_major+10*_minor;

  *medok= MED_TRUE;
  /* if ( (_major > 2) || ((_major == 2) && (_minor > 1)) ) *medok = MED_TRUE; */
  if ( _fversionMM <  220 ) *medok = MED_FALSE;
  /* Si le mineur du fichier est plus récent que celui de la bibliothèque, la bibliothèque n'est pas
   capable de relire le modèle.*/
  if ( _fversionMM > 100*MED_NUM_MAJEUR+10*MED_NUM_MINEUR ) *medok = MED_FALSE;
  
  _ret = 0;
 ERROR:

  if (_id > 0)
    if (_MEDdatagroupFermer(_id) < 0) {
      MED_ERR_(_ret,MED_ERR_CLOSE,MED_ERR_DATAGROUP,MED_INFOS);
      _ret = -1;
    }
  
  if (_fid > 0)
    if (MEDfileClose(_fid) < 0) {
      MED_ERR_(_ret,MED_ERR_CLOSE,MED_ERR_FILE,filename);
      _ret = -1;
    }

  return _ret;
}
