
Gem::Specification.new do |s|
  s.name    = "ifusb"
  s.version = "0.1.0"
  s.summary = "ifusb for Ruby"
  s.author  = "Julien Boulnois"

  s.files = Dir.glob("ext/*.{c,rb}") +
            Dir.glob("lib/*.rb")

  s.extensions << "ext/extconf.rb"

  s.add_development_dependency "rake-compiler"
end
