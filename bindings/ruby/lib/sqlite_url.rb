require "version"

module SqliteUrl
  class Error < StandardError; end
  def self.url_loadable_path
    File.expand_path('../url0', __FILE__)
  end
  def self.load(db)
    db.load_extension(self.url_loadable_path)
  end
end
