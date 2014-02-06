/*
* This file is part of Wakanda software, licensed by 4D under
*  (i) the GNU General Public License version 3 (GNU GPL v3), or
*  (ii) the Affero General Public License version 3 (AGPL v3) or
*  (iii) a commercial license.
* This file remains the exclusive property of 4D and/or its licensors
* and is protected by national and international legislations.
* In any event, Licensee's compliance with the terms and conditions
* of the applicable license constitutes a prerequisite to any use of this file.
* Except as otherwise expressly stated in the applicable license,
* such license does not include any other license or rights on this file,
* 4D's and/or its licensors' trademarks and/or other proprietary rights.
* Consequently, no title, copyright or other proprietary rights
* other than those specified in the applicable license is granted.
*/
RestDirectoryAccess = {};

RestDirectoryAccess.loginByKey = function(userName, key, timeout)
{
	return loginByKey(userName, key, timeout);
}

RestDirectoryAccess.login = function(userName, password, timeout)
{
	return loginByPassword(userName, password, timeout);
}

RestDirectoryAccess.loginByPassword = RestDirectoryAccess.login;

RestDirectoryAccess.logout = function()
{
	logout();
	return true;
}

RestDirectoryAccess.currentUser = function() {
	var user = currentUser();
	if (user == null)
		return null;
	else {
		if (user.ID === "00000000000000000000000000000000")
			return null;
		else {
			return {
				userName: user.name,
				fullName: user.fullName,
				ID: user.ID
			}
		}
	}
}

RestDirectoryAccess.currentUserBelongsTo = function(group) {
    var session = currentSession();
    if (session == null)
        return false;
    else
        return session.belongsTo(group);
}

