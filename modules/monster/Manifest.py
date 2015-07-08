def __helper():
  files = [ 
    "monster.vhd",
    "monster_pkg.vhd",
    "monster_iodir.vhd",
    ]
  if syn_device[:1] == "5":    files.extend(["monster5.qip"])
  # if syn_device[:4] == "ep2a": files.extend(["monster2.qip"])
  return files

files = __helper()
