target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_device = "5agxma3d4f"
syn_grade = "c5"
syn_package = "27"
syn_top = "vetar5"
syn_project = "vetar5"

quartus_preflow = "vetar5.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_vetar5/control", 
  ]
}

