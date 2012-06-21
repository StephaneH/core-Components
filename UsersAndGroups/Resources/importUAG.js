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
function exportUAG(uagStore, filePath) {
    var file = File(filePath);
    var User = uagStore.User;
    var Group = uagStore.Group;
    var Usergroup = uagStore.Usergroup;
    var Groupgroup = uagStore.Groupgroup;

    if (file.exists)
        file.remove();

    var result = {
        user: [],
        group: []
    };

    var users = result.user;
    var groups = result.group;

    User.forEach(function(user) {
        var oneUser = { name: user.name, ID: user.ID, fullName: user.fullName, password: user.password };
        users.push(oneUser);
    });

    Group.forEach(function(group) {
        var oneGroup = { name: group.name, ID: group.ID, fullName: group.fullName };
        var include = [];
        var belongsTo = [];

        group.users.forEach(function(user) {
            var oneInclude = { user: user.name, userID: user.ID };
            include.push(oneInclude);
        });

        group.children.forEach(function(child) {
            var oneInclude = { group: child.name, groupID: child.ID };
            include.push(oneInclude);

        });

        if (include.length > 0)
            oneGroup.include = include;
        if (belongsTo.length > 0)
            oneGroup.belongsTo = belongsTo;

        groups.push(oneGroup);
    });

    var jsontext = JSON.stringify(result);
    var xmltext = JSONToXml(jsontext, "json-bag", "directory");
    var oktowrite = true;
    if (file.exists) {
        oktowrite = file.remove();
    }
    if (oktowrite) {
        saveText(xmltext, file);
    }
}

function importUAG(uagStore, filePath) {
    var file = File(filePath);
    var User = uagStore.User;
    var Group = uagStore.Group;
    var Usergroup = uagStore.Usergroup;
    var Groupgroup = uagStore.Groupgroup;
    var xmltext = loadText(file);
    if (xmltext != null) {
        var jsontext = XmlToJSON(xmltext, "json-bag", "directory");
        var uag = JSON.parse(jsontext);
        trace(JSON.stringify(uag) + "\n");
        var users = uag.user;
        var groups = uag.group;
        if (users != null) // get list of users
        {
            var nbusers = users.length;
            for (var i = 0; i < nbusers; i++) {
                var oneUser = users[i];
                if (oneUser.name != null) {
                    var user = new User();
                    user.name = oneUser.name;
                    if (oneUser.fullName == null)
                        user.fullName = oneUser.name;
                    else
                        user.fullName = oneUser.fullName;
                    user.password = oneUser.password || null;
                    if (oneUser.ID != null)
                        user.ID = oneUser.ID;
                    user.save();
                }
            }
        }

        if (groups != null) // get list of groups
        {
            var nbgroups = groups.length;
            for (var i = 0; i < nbgroups; i++) {
                var oneGroup = groups[i];
                if (oneGroup.name != null) {
                    var group = new Group();
                    group.name = oneGroup.name;
                    if (oneGroup.fullName == null)
                        group.fullName = oneGroup.name;
                    else
                        group.fullName = oneGroup.fullName;
                    if (oneGroup.ID != null)
                        group.ID = oneGroup.ID;
                    group.save();
                }
            }
        }

        if (users != null) // go through users and find groups they belong to
        {
            var nbusers = users.length;
            for (var i = 0; i < nbusers; i++) {
                var oneUser = users[i];
                trace("checking belongsTo for " + oneUser.name + "\n");
                if (oneUser.name != null) {
                    var user;
                    if (oneUser.ID == null)
                        user = User({
                            name: oneUser.name
                        });
                    else
                        user = User(oneUser.ID);
                    if (user != null) {
                        trace("found user " + oneUser.name + "\n");
                        var belongsTo = oneUser.belongsTo;
                        if (belongsTo != null) {
                            trace(user.name + " belongs to " + belongsTo.length + " groups" + "\n");
                            for (var j = 0; j < belongsTo.length; j++) {
                                var oneLine = belongsTo[j];
                                trace(user.name + " is in " + oneLine.group + "\n");
                                var owner;
                                if (oneLine.groupID == null)
                                    owner = Group({
                                        name: oneLine.group
                                    });
                                else
                                    owner = Group(oneLine.groupID);
                                if (owner != null) {
                                    var usergroup = new Usergroup({
                                        user: user,
                                        group: owner
                                    });
                                    usergroup.save();
                                }
                            }
                        }
                    }
                }
            }
        }

        if (groups != null) // go through groups and find other groups they belong to and other groups they include
        {
            var nbgroups = groups.length;
            for (var i = 0; i < nbgroups; i++) {
                var oneGroup = groups[i];
                if (oneGroup.name != null) {
                    var group;
                    if (oneGroup.ID == null)
                        group = Group({
                            name: oneGroup.name
                        });
                    else
                        group = Group(oneGroup.ID);
                    if (group != null) {
                        var belongsTo = oneGroup.belongsTo; // find other groups that group belong to
                        if (belongsTo != null) {
                            for (var j = 0; j < belongsTo.length; j++) {
                                var oneLine = belongsTo[j];
                                var owner;
                                if (oneLine.groupID == null)
                                    owner = Group({
                                        name: oneLine.group
                                    });
                                else
                                    owner = Group(oneLine.groupID);
                                if (owner != null) {
                                    var groupgroup = new Groupgroup({
                                        child: group,
                                        parent: owner
                                    });
                                    groupgroup.save();
                                }
                            }
                        }
                        var include = oneGroup.include; // find users or other groups that group includes
                        if (include != null) {
                            for (var j = 0; j < include.length; j++) {
                                var oneLine = include[j];
                                var included;
                                if (oneLine.user != null || oneLine.userID != null) // find included user
                                {
                                    if (oneLine.userID == null)
                                        included = User({
                                            name: oneLine.user
                                        });
                                    else
                                        included = User(oneLine.userID);
                                    if (included != null) {
                                        var usergroup = new Usergroup({
                                            user: included,
                                            group: group
                                        });
                                        usergroup.save();
                                    }
                                }
                                else // find included group
                                {
                                    if (oneLine.groupID == null)
                                        included = Group({
                                            name: oneLine.group
                                        });
                                    else
                                        included = Group(oneLine.groupID);
                                    if (included != null) {
                                        var groupgroup = new Groupgroup({
                                            child: included,
                                            parent: group
                                        });
                                        groupgroup.save();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
